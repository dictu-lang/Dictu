#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "memory.h"
#include "value.h"
#include "vm.h"

void initValueArray(ValueArray *array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeValueArray(VM *vm, ValueArray *array, Value value) {
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(vm, array->values, Value,
                                   oldCapacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(VM *vm, ValueArray *array) {
    FREE_ARRAY(vm, Value, array->values, array->capacity);
    initValueArray(array);
}

void initSetValues(ObjSet *set, uint32_t capacity) {
    set->capacity = capacity;
    set->count = 0;
    set->items = calloc(capacity, sizeof(*set->items));
}

void insertSet(VM *vm, ObjSet *set, Value value) {
    if (!IS_STRING(value)) {
        printf("Sets can only store string values!"); // TODO: To be revised as more values support hashing
        return;
    }

    ObjString *string = AS_STRING(value);

    // If the value is already in the set, exit
    if (searchSetMarkActive(set, string)) {
        return;
    }

    int index = string->hash % set->capacity;

    setItem *item = ALLOCATE(vm, setItem, sizeof(setItem));

    if (set->count * 100 / set->capacity >= 60) {
        resizeSet(vm, set, true);
    }

    item->item = string;
    item->hash = string->hash;
    item->deleted = false;

    while (set->items[index] && strcmp(set->items[index]->item->chars, string->chars) != 0) {
        index++;
        if (index == set->capacity) {
            index = 0;
        }
    }

    set->items[index] = item;
    set->count++;
}

bool searchSet(ObjSet *set, ObjString *string) {
    int index = string->hash % set->capacity;

    while (set->items[index] && strcmp(set->items[index]->item->chars, string->chars) != 0) {
        index++;
        if (index == set->capacity) {
            index = 0;
        }
    }

    return set->items[index] && !set->items[index]->deleted;
}

bool searchSetMarkActive(ObjSet *set, ObjString *string) {
    int index = string->hash % set->capacity;

    while (set->items[index] && strcmp(set->items[index]->item->chars, string->chars) != 0) {
        index++;
        if (index == set->capacity) {
            index = 0;
        }
    }

    if (set->items[index]) {
        // If we found the value but it's been "deleted" mark it as active
        if (set->items[index]->deleted) {
            set->items[index]->deleted = false;
            set->count++;
        }
        return true;
    }

    return false;
}

void resizeSet(VM *vm, ObjSet *set, bool grow) {
    int newSize;

    if (grow)
        newSize = set->capacity << 1;
    else
        newSize = set->capacity >> 1;

    setItem **items = (setItem **)ALLOCATE(vm, setItem, newSize);

    for (int j = 0; j < set->capacity; ++j) {
        if (!set->items[j])
            continue;

        if (set->items[j]->deleted) {
            freeSetValue(set->items[j]);
            continue;
        }

        int index = set->items[j]->hash % newSize;

        while (items[index]) {
            index = (index + 1) % newSize; // Handles wrap around indexes
        }

        items[index] = set->items[j];
    }

    free(set->items);

    set->capacity = newSize;
    set->items = items;
}

// Calling function needs to free memory
char *valueToString(Value value) {
    if (IS_BOOL(value)) {
        char *str = AS_BOOL(value) ? "true" : "false";
        char *boolString = malloc(sizeof(char) * (strlen(str) + 1));
        snprintf(boolString, strlen(str) + 1, "%s", str);
        return boolString;
    } else if (IS_NIL(value)) {
        char *nilString = malloc(sizeof(char) * 4);
        snprintf(nilString, 4, "%s", "nil");
        return nilString;
    } else if (IS_NUMBER(value)) {
        double number = AS_NUMBER(value);
        int numberStringLength = snprintf(NULL, 0, "%.15g", number) + 1;
        char *numberString = malloc(sizeof(char) * numberStringLength);
        snprintf(numberString, numberStringLength, "%.15g", number);
        return numberString;
    } else if (IS_OBJ(value)) {
        return objectToString(value);
    }

    char *unknown = malloc(sizeof(char) * 8);
    snprintf(unknown, 8, "%s", "unknown");
    return unknown;
}

void printValue(Value value) {
    char *output = valueToString(value);
    printf("%s", output);
    free(output);
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

static bool dictComparison(Value a, Value b) {
    ObjDict *dict = AS_DICT(a);
    ObjDict *dictB = AS_DICT(b);

    // Different lengths, not the same
    if (dict->items.count != dictB->items.count)
        return false;

    // Lengths are the same, and dict 1 has 0 length
    // therefore both are empty
    if (dict->items.count == 0)
        return true;

    for (int i = 0; i <= dict->items.capacityMask; ++i) {
        Entry *item = &dict->items.entries[i];
        Entry *itemB = &dictB->items.entries[i];

        if (item->key == NULL || itemB->key == NULL) {
            continue;
        }

        if (!valuesEqual(OBJ_VAL(item->key), OBJ_VAL(itemB->key)))
            return false;

        if (!valuesEqual(item->value, itemB->value))
            return false;
    }

    return true;
}

static bool setComparison(Value a, Value b) {
    ObjSet *set = AS_SET(a);
    ObjSet *setB = AS_SET(b);

    if (set->count != setB->count)
        return false;

    for (int i = 0; i < set->count; ++i) {
        if (set->items[i]!= setB->items[i])
            return false;
    }

    return true;
}

bool valuesEqual(Value a, Value b) {
#ifdef NAN_TAGGING

    if (IS_OBJ(a) && IS_OBJ(b)) {
        if (AS_OBJ(a)->type == OBJ_LIST && AS_OBJ(b)->type == OBJ_LIST) {
            return listComparison(a, b);
        }

        if (AS_OBJ(a)->type == OBJ_DICT && AS_OBJ(b)->type == OBJ_DICT) {
            return dictComparison(a, b);
        }

        if (AS_OBJ(a)->type == OBJ_SET && AS_OBJ(b)->type == OBJ_SET) {
            return setComparison(a, b);
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
