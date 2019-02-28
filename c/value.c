#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "memory.h"
#include "value.h"

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

// TODO: Implement adding to hash table
// TODO: Implement searching hash table
// TODO: Implement removing a key

void initDictValues(ObjDict *dict) {
    dict->capacity = 10;
    dict->count = 0;

    for (int i = 0; i < dict->capacity; i++)
    {
        dict->items[i] = NULL;
    }
}

void freeDictValue(dictItems *dictItem) {
    free(dictItem->key);
    free(dictItem->item);
    free(dictItem);
}

void freeDict(ObjDict *dict) {
    for (int i = 0; i < dict->size; i++) {
        dictItems* item = dict->items[i];
        if (item != NULL) {
            freeDictValue(item);
        }
    }
    free(dict->items);
    free(dict);
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

bool valuesEqual(Value a, Value b) {
#ifdef NAN_TAGGING
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
