#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "memory.h"
#include "value.h"
#include "vm.h"

static Obj *allocateObject(size_t size, ObjType type, bool isList) {
    Obj *object = (Obj *) reallocate(NULL, 0, size);
    object->type = type;
    object->isDark = false;

    if (!isList) {
        object->next = vm.objects;
        vm.objects = object;
    } else {
        object->next = vm.listObjects;
        vm.listObjects = object;
    }

#ifdef DEBUG_TRACE_GC
    printf("%p allocate %ld for %d\n", (void *)object, size, type);
#endif

    return object;
}

#define ALLOCATE_OBJ_LIST(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType, true)

void initValueArray(ValueArray *array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeValueArray(ValueArray *array, Value value) {
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(array->values, Value,
                                   oldCapacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray *array) {
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}

ObjDict *initDictValues(uint32_t capacity) {
    ObjDict *dict = ALLOCATE_OBJ_LIST(ObjDict, OBJ_DICT);
    dict->capacity = capacity;
    dict->count = 0;
    dict->items = malloc(capacity * sizeof(*dict->items));

    for (int i = 0; i < dict->capacity; i++) {
        dict->items[i] = NULL;
    }

    return dict;
}

static uint32_t hash(char *str) {
    uint32_t hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}

void insertDict(ObjDict *dict, char *key, Value value) {
    uint32_t hashValue = hash(key);
    int index = hashValue % dict->capacity;

    char *key_m = ALLOCATE(char, strlen(key) + 1);

    if (!key_m) {
        printf("ERROR!");
        return;
    }

    strcpy(key_m, key);

    dictItem *item = ALLOCATE(dictItem, sizeof(dictItem));

    if (!item) {
        printf("ERROR!");
        return;
    }

    item->key = key_m;
    item->item = value;
    item->deleted = false;
    item->hash = hashValue;

    if (dict->count * 100 / dict->capacity >= 70) {
        resizeDict(dict);
    }

    while (index < dict->capacity &&
           dict->items[index] && !dict->items[index]->deleted && strcmp(dict->items[index]->key, key) != 0) {
        index++;
        if (index == dict->capacity)
            index = 0;
    }


    if (dict->items[index]) {
        free(dict->items[index]->key);
        free(dict->items[index]);
    }

    dict->items[index] = item;
    dict->count++;
}

void resizeDict(ObjDict *dict) {
    int newSize = dict->capacity << 1; // Grow by a factor of 2

    dictItem **items = malloc(newSize * sizeof(*dict->items));

    for (int i = 0; i < newSize; ++i) {
        items[i] = NULL;
    }

    for (int j = 0; j < dict->capacity; ++j) {
        if (!dict->items[j])
            continue;

        if (dict->items[j]->deleted) {
            freeDictValue(dict->items[j]);
            continue;
        }

        int index = dict->items[j]->hash % newSize;

        while (index < newSize && items[index]) {
            index++;
            if (index == newSize)
                index = 0;
        }

        items[index] = dict->items[j];
    }

    free(dict->items);

    dict->capacity = newSize;
    dict->items = items;
}

Value searchDict(ObjDict *dict, char *key) {
    int index = hash(key) % dict->capacity;

    if (!dict->items[index])
        return NIL_VAL;

    if (strcmp(dict->items[index]->key, key) == 0) {
        return dict->items[index]->item;
    }

    return NIL_VAL;
}

void printValue(Value value) {
#ifdef NAN_TAGGING
    if (IS_BOOL(value)) {
        printf(AS_BOOL(value) ? "true" : "false");
    } else if (IS_NIL(value)) {
        printf("nil");
    } else if (IS_NUMBER(value)) {
        printf("%.15g", AS_NUMBER(value));
    } else if (IS_OBJ(value)) {
        printObject(value);
    }
#else
    switch (value.type) {
      case VAL_BOOL:   printf(AS_BOOL(value) ? "true" : "false"); break;
      case VAL_NIL:    printf("nil"); break;
      case VAL_NUMBER: printf("%g", AS_NUMBER(value)); break;
      case VAL_OBJ:    printObject(value); break;
    }
#endif
}

static bool dictComparison(Value a, Value b) {
    ObjDict *dict = AS_DICT(a);
    ObjDict *dictB = AS_DICT(b);

    // Different lengths, not the same
    if (dict->capacity != dictB->capacity)
        return false;

    // Lengths are the same, and dict 1 has 0 length
    // therefore both are empty
    if (dict->count == 0)
        return true;

    for (int i = 0; i < dict->capacity; ++i) {
        dictItem *item = dict->items[i];
        dictItem *itemB = dictB->items[i];

        if (!item || !itemB) {
            continue;
        }

        if (strcmp(item->key, itemB->key) != 0)
            return false;

        if (!valuesEqual(item->item, itemB->item))
            return false;
    }

    return true;
}

static bool listComparison(Value a, Value b) {
    ObjList *list = AS_LIST(a);
    ObjList *listB = AS_LIST(b);

    if (list->values.count != listB->values.count)
        return false;

    for (int i = 0; i < list->values.count; ++i) {
        if (!valuesEqual(list->values.values[i], listB->values.values[i]))
            return false;
    }

    return true;
}

bool valuesEqual(Value a, Value b) {
#ifdef NAN_TAGGING

    if (IS_OBJ(a) && IS_OBJ(b)) {
        if (AS_OBJ(a)->type == OBJ_DICT && AS_OBJ(b)->type == OBJ_DICT) {
            return dictComparison(a, b);
        }

        if (AS_OBJ(a)->type == OBJ_LIST && AS_OBJ(b)->type == OBJ_LIST) {
            return listComparison(a, b);
        }
    }

    return a == b;
#else
    if (a.type != b.type) return false;

    switch (a.type) {
      case VAL_BOOL:   return AS_BOOL(a) == AS_BOOL(b);
      case VAL_NIL:    return true;
      case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
      case VAL_OBJ:
        return AS_OBJ(a) == AS_OBJ(b);
    }
#endif
}
