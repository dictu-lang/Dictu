#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "memory.h"
#include "value.h"
#include "vm.h"

#define TABLE_MAX_LOAD 0.75
#define TABLE_MIN_LOAD 0.25

void initValueArray(ValueArray *array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeValueArray(DictuVM *vm, ValueArray *array, Value value) {
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(vm, array->values, Value,
                                   oldCapacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(DictuVM *vm, ValueArray *array) {
    FREE_ARRAY(vm, Value, array->values, array->capacity);
    initValueArray(array);
}

static inline uint32_t hashBits(uint64_t hash) {
    // From v8's ComputeLongHash() which in turn cites:
    // Thomas Wang, Integer Hash Functions.
    // http://www.concentric.net/~Ttwang/tech/inthash.htm
    hash = ~hash + (hash << 18);  // hash = (hash << 18) - hash - 1;
    hash = hash ^ (hash >> 31);
    hash = hash * 21;  // hash = (hash + (hash << 2)) + (hash << 4);
    hash = hash ^ (hash >> 11);
    hash = hash + (hash << 6);
    hash = hash ^ (hash >> 22);
    return (uint32_t) (hash & 0x3fffffff);
}

static uint32_t hashObject(Obj *object) {
    switch (object->type) {
        case OBJ_STRING: {
            return ((ObjString *) object)->hash;
        }

            // Should never get here
        default: {
#ifdef DEBUG_PRINT_CODE
            printf("Object: ");
            printValue(OBJ_VAL(object));
            printf(" not hashable!\n");
            exit(1);
#endif
            return -1;
        }
    }
}

static uint32_t hashValue(Value value) {
    if (IS_OBJ(value)) {
        return hashObject(AS_OBJ(value));
    }

    return hashBits(value);
}

static DictItem *findDictEntry(DictItem *entries, int capacityMask,
                               Value key) {
    uint32_t index = hashValue(key) & capacityMask;
    DictItem *tombstone = NULL;

    for (;;) {
        DictItem *entry = &entries[index];

        if (IS_EMPTY(entry->key)) {
            if (IS_NIL(entry->value)) {
                // Empty entry.
                return tombstone != NULL ? tombstone : entry;
            } else {
                // We found a tombstone.
                if (tombstone == NULL) tombstone = entry;
            }
        } else if (valuesEqual(key, entry->key)) {
            // We found the key.
            return entry;
        }

//        printf("%d - ", index);
        index = (index + 1) & capacityMask;
//        printf("%d - mask: %d\n", index, capacityMask);
    }
}

bool dictGet(ObjDict *dict, Value key, Value *value) {
    if (dict->count == 0) return false;

    DictItem *entry = findDictEntry(dict->entries, dict->capacityMask, key);
    if (IS_EMPTY(entry->key)) return false;

    *value = entry->value;
    return true;
}

static void adjustDictCapacity(DictuVM *vm, ObjDict *dict, int capacityMask) {
    DictItem *entries = ALLOCATE(vm, DictItem, capacityMask + 1);
    for (int i = 0; i <= capacityMask; i++) {
        entries[i].key = EMPTY_VAL;
        entries[i].value = NIL_VAL;
    }

    dict->count = 0;

    for (int i = 0; i <= dict->capacityMask; i++) {
        DictItem *entry = &dict->entries[i];
        if (IS_EMPTY(entry->key)) continue;

        DictItem *dest = findDictEntry(entries, capacityMask, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        dict->count++;
    }

    FREE_ARRAY(vm, DictItem, dict->entries, dict->capacityMask + 1);
    dict->entries = entries;
    dict->capacityMask = capacityMask;
}

bool dictSet(DictuVM *vm, ObjDict *dict, Value key, Value value) {
    if (dict->count + 1 > (dict->capacityMask + 1) * TABLE_MAX_LOAD) {
        // Figure out the new table size.
        int capacityMask = GROW_CAPACITY(dict->capacityMask + 1) - 1;
        adjustDictCapacity(vm, dict, capacityMask);
    }

    DictItem *entry = findDictEntry(dict->entries, dict->capacityMask, key);
    bool isNewKey = IS_EMPTY(entry->key);

    entry->key = key;
    entry->value = value;

    if (isNewKey) {
        dict->activeCount++;
        dict->count++;
    }

    return isNewKey;
}

bool dictDelete(DictuVM *vm, ObjDict *dict, Value key) {
    if (dict->count == 0) return false;

    DictItem *entry = findDictEntry(dict->entries, dict->capacityMask, key);
    if (IS_EMPTY(entry->key)) return false;

    // Place a tombstone in the entry.
    dict->activeCount--;
    entry->key = EMPTY_VAL;
    entry->value = BOOL_VAL(true);

    if (dict->count - 1 < dict->capacityMask * TABLE_MIN_LOAD) {
        // Figure out the new table size.
        int capacityMask = SHRINK_CAPACITY(dict->capacityMask + 1) - 1;
        adjustDictCapacity(vm, dict, capacityMask);
    }

    return true;
}

void grayDict(DictuVM *vm, ObjDict *dict) {
    for (int i = 0; i <= dict->capacityMask; i++) {
        DictItem *entry = &dict->entries[i];
        grayValue(vm, entry->key);
        grayValue(vm, entry->value);
    }
}

static SetItem *findSetEntry(SetItem *entries, int capacityMask,
                             Value value) {
    uint32_t index = hashValue(value) & capacityMask;
    SetItem *tombstone = NULL;

    for (;;) {
        SetItem *entry = &entries[index];

        if (IS_EMPTY(entry->value)) {
            if (!entry->deleted) {
                // Empty entry.
                return tombstone != NULL ? tombstone : entry;
            } else {
                // We found a tombstone.
                if (tombstone == NULL) tombstone = entry;
            }
        } else if (valuesEqual(value, entry->value)) {
            // We found the key.
            return entry;
        }

        index = (index + 1) & capacityMask;
    }
}

bool setGet(ObjSet *set, Value value) {
    if (set->count == 0) return false;

    SetItem *entry = findSetEntry(set->entries, set->capacityMask, value);
    if (IS_EMPTY(entry->value) || entry->deleted) return false;

    return true;
}

static void adjustSetCapacity(DictuVM *vm, ObjSet *set, int capacityMask) {
    SetItem *entries = ALLOCATE(vm, SetItem, capacityMask + 1);
    for (int i = 0; i <= capacityMask; i++) {
        entries[i].value = EMPTY_VAL;
        entries[i].deleted = false;
    }

    set->count = 0;

    for (int i = 0; i <= set->capacityMask; i++) {
        SetItem *entry = &set->entries[i];
        if (IS_EMPTY(entry->value) || entry->deleted) continue;

        SetItem *dest = findSetEntry(entries, capacityMask, entry->value);
        dest->value = entry->value;
        set->count++;
    }

    FREE_ARRAY(vm, SetItem, set->entries, set->capacityMask + 1);
    set->entries = entries;
    set->capacityMask = capacityMask;
}

bool setInsert(DictuVM *vm, ObjSet *set, Value value) {
    if (set->count + 1 > (set->capacityMask + 1) * TABLE_MAX_LOAD) {
        // Figure out the new table size.
        int capacityMask = GROW_CAPACITY(set->capacityMask + 1) - 1;
        adjustSetCapacity(vm, set, capacityMask);
    }

    SetItem *entry = findSetEntry(set->entries, set->capacityMask, value);
    bool isNewKey = IS_EMPTY(entry->value) || entry->deleted;
    entry->value = value;
    entry->deleted = false;

    if (isNewKey) set->count++;

    return isNewKey;
}

bool setDelete(DictuVM *vm, ObjSet *set, Value value) {
    if (set->count == 0) return false;

    SetItem *entry = findSetEntry(set->entries, set->capacityMask, value);
    if (IS_EMPTY(entry->value)) return false;

    // Place a tombstone in the entry.
    set->count--;
    entry->deleted = true;

    if (set->count - 1 < set->capacityMask * TABLE_MIN_LOAD) {
        // Figure out the new table size.
        int capacityMask = SHRINK_CAPACITY(set->capacityMask + 1) - 1;
        adjustSetCapacity(vm, set, capacityMask);
    }

    return true;
}

void graySet(DictuVM *vm, ObjSet *set) {
    for (int i = 0; i <= set->capacityMask; i++) {
        SetItem *entry = &set->entries[i];
        grayValue(vm, entry->value);
    }
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

// Calling function needs to free memory
char *valueTypeToString(DictuVM *vm, Value value, int *length) {
#define CONVERT(typeString, size)                     \
    do {                                              \
        char *string = ALLOCATE(vm, char, size + 1);  \
        memcpy(string, #typeString, size);            \
        string[size] = '\0';                          \
        *length = size;                               \
        return string;                                \
    } while (false)

#define CONVERT_VARIABLE(typeString, size)            \
    do {                                              \
        char *string = ALLOCATE(vm, char, size + 1);  \
        memcpy(string, typeString, size);             \
        string[size] = '\0';                          \
        *length = size;                               \
        return string;                                \
    } while (false)


    if (IS_BOOL(value)) {
        CONVERT(bool, 4);
    } else if (IS_NIL(value)) {
        CONVERT(nil, 3);
    } else if (IS_NUMBER(value)) {
        CONVERT(number, 6);
    } else if (IS_OBJ(value)) {
        switch (OBJ_TYPE(value)) {
            case OBJ_CLASS: {
                switch (AS_CLASS(value)->type) {
                    case CLASS_DEFAULT:
                    case CLASS_ABSTRACT: {
                        CONVERT(class, 5);
                    }
                    case CLASS_TRAIT: {
                        CONVERT(trait, 5);
                    }
                }

                break;
            }
            case OBJ_ENUM: {
                CONVERT(enum, 4);
            }
            case OBJ_MODULE: {
                CONVERT(module, 6);
            }
            case OBJ_INSTANCE: {
                ObjString *className = AS_INSTANCE(value)->klass->name;

                CONVERT_VARIABLE(className->chars, className->length);
            }
            case OBJ_BOUND_METHOD: {
                CONVERT(method, 6);
            }
            case OBJ_CLOSURE:
            case OBJ_FUNCTION: {
                CONVERT(function, 8);
            }
            case OBJ_STRING: {
                CONVERT(string, 6);
            }
            case OBJ_LIST: {
                CONVERT(list, 4);
            }
            case OBJ_DICT: {
                CONVERT(dict, 4);
            }
            case OBJ_SET: {
                CONVERT(set, 3);
            }
            case OBJ_NATIVE: {
                CONVERT(native, 6);
            }
            case OBJ_FILE: {
                CONVERT(file, 4);
            }
            case OBJ_RESULT: {
                CONVERT(result, 6);
            }
            default:
                break;
        }
    }

    CONVERT(unknown, 7);
#undef CONVERT
#undef CONVERT_VARIABLE
}

void printValue(Value value) {
    char *output = valueToString(value);
    printf("%s", output);
    free(output);
}

void printValueError(Value value) {
    char *output = valueToString(value);
    fprintf(stderr, "%s", output);
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
    if (dict->activeCount != dictB->activeCount)
        return false;

    // Lengths are the same, and dict 1 has 0 length
    // therefore both are empty
    if (dict->activeCount == 0)
        return true;

    for (int i = 0; i <= dict->capacityMask; ++i) {
        DictItem *item = &dict->entries[i];

        if (IS_EMPTY(item->key))
            continue;

        Value value;
        // Check if key from dict A is in dict B
        if (!dictGet(dictB, item->key, &value)) {
            // Key doesn't exist
            return false;
        }

        // Key exists
        if (!valuesEqual(item->value, value)) {
            // Values don't equal
            return false;
        }
    }

    return true;
}

static bool setComparison(Value a, Value b) {
    ObjSet *set = AS_SET(a);
    ObjSet *setB = AS_SET(b);

    // Different lengths, not the same
    if (set->count != setB->count)
        return false;

    // Lengths are the same, and dict 1 has 0 length
    // therefore both are empty
    if (set->count == 0)
        return true;

    for (int i = 0; i <= set->capacityMask; ++i) {
        SetItem *item = &set->entries[i];

        if (IS_EMPTY(item->value) || item->deleted)
            continue;

        // Check if key from dict A is in dict B
        if (!setGet(setB, item->value)) {
            // Key doesn't exist
            return false;
        }
    }

    return true;
}

bool valuesEqual(Value a, Value b) {
    if (IS_OBJ(a) && IS_OBJ(b)) {
        if (AS_OBJ(a)->type != AS_OBJ(b)->type) return false;

        switch (AS_OBJ(a)->type) {
            case OBJ_LIST: {
                return listComparison(a, b);
            }

            case OBJ_DICT: {
                return dictComparison(a, b);
            }

            case OBJ_SET: {
                return setComparison(a, b);
            }

                // Pass through
            default:
                break;
        }
    }

    return a == b;
}

bool compareStringLess(Value a, Value b) {
    // Swapping as it's a stack
    ObjString *secondString = AS_STRING(a);
    ObjString *firstString = AS_STRING(b);

    int i;

    for (i = 0; firstString->chars[i] != '\0' || secondString->chars[i] != '\0'; i++) {
        if (firstString->chars[i] < secondString->chars[i]) {
            return true;
        }

        if (firstString->chars[i] > secondString->chars[i]) {
            return false;
        }
    }

    if (secondString->chars[i] != '\0') {
        return true;
    }

    return false;
}

bool compareStringGreater(Value a, Value b) {
    // Swapping as it's a stack
    ObjString *secondString = AS_STRING(a);
    ObjString *firstString = AS_STRING(b);

    int i;

    for (i = 0; firstString->chars[i] != '\0' || secondString->chars[i] != '\0'; i++) {
        if (firstString->chars[i] > secondString->chars[i]) {
            return true;
        }

        if (firstString->chars[i] < secondString->chars[i]) {
            return false;
        }
    }

    if (firstString->chars[i] != '\0') {
        return true;
    }

    return false;
}