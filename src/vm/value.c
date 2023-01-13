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

static uint32_t hashObject(DictuVM *vm, Obj *object) { 
    switch (object->type) {
        case OBJ_STRING: {
            UNUSED(vm);
            return ((ObjString *) object)->hash;
        }

            // Should never get here
        default: {
#ifdef DEBUG_PRINT_CODE
            printf("Object: ");
            printValue(vm, OBJ_VAL(object));
            printf(" not hashable!\n");
            exit(1);
#endif
            return -1;
        }
    }
}

static uint32_t hashValue(DictuVM *vm, Value value) {
    if (IS_OBJ(value)) {
        return hashObject(vm, AS_OBJ(value));
    }

    return hashBits(value);
}

bool dictGet(DictuVM *vm, ObjDict *dict, Value key, Value *value) {
    if (dict->count == 0) return false;

    DictItem *entry;
    uint32_t index = hashValue(vm, key) & dict->capacityMask;
    uint32_t psl = 0;

    for (;;) {
        entry = &dict->entries[index];

        if (IS_EMPTY(entry->key) || psl > entry->psl) {
            return false;
        }

        if (valuesEqual(vm, key, entry->key)) {
            break;
        }

        index = (index + 1) & dict->capacityMask;
        psl++;
    }

    *value = entry->value;
    return true;
}

static void adjustDictCapacity(DictuVM *vm, ObjDict *dict, int capacityMask) {
    DictItem *entries = ALLOCATE(vm, DictItem, capacityMask + 1);
    for (int i = 0; i <= capacityMask; i++) {
        entries[i].key = EMPTY_VAL;
        entries[i].value = NIL_VAL;
        entries[i].psl = 0;
    }

    DictItem *oldEntries = dict->entries;
    int oldMask = dict->capacityMask;

    dict->count = 0;
    dict->entries = entries;
    dict->capacityMask = capacityMask;

    for (int i = 0; i <= oldMask; i++) {
        DictItem *entry = &oldEntries[i];
        if (IS_EMPTY(entry->key)) continue;

        dictSet(vm, dict, entry->key, entry->value);
    }

    FREE_ARRAY(vm, DictItem, oldEntries, oldMask + 1);
}

bool dictSet(DictuVM *vm, ObjDict *dict, Value key, Value value) {
    if (dict->count + 1 > (dict->capacityMask + 1) * TABLE_MAX_LOAD) {
        // Figure out the new table size.
        int capacityMask = GROW_CAPACITY(dict->capacityMask + 1) - 1;
        adjustDictCapacity(vm, dict, capacityMask);
    }

    uint32_t index = hashValue(vm, key) & dict->capacityMask;
    DictItem *bucket;
    bool isNewKey = false;

    DictItem entry;
    entry.key = key;
    entry.value = value;
    entry.psl = 0;

    for (;;) {
        bucket = &dict->entries[index];

        if (IS_EMPTY(bucket->key)) {
            isNewKey = true;
            break;
        } else {
            if (valuesEqual(vm, key, bucket->key)) {
                break;
            }

            if (entry.psl > bucket->psl) {
                isNewKey = true;
                DictItem tmp = entry;
                entry = *bucket;
                *bucket = tmp;
            }
        }

        index = (index + 1) & dict->capacityMask;
        entry.psl++;
    }

    *bucket = entry;
    if (isNewKey) dict->count++;
    return isNewKey;
}

bool dictDelete(DictuVM *vm, ObjDict *dict, Value key) {
    if (dict->count == 0) return false;

    int capacityMask = dict->capacityMask;
    uint32_t index = hashValue(vm, key) & capacityMask;
    uint32_t psl = 0;
    DictItem *entry;

    for (;;) {
        entry = &dict->entries[index];

        if (IS_EMPTY(entry->key) || psl > entry->psl) {
            return false;
        }

        if (valuesEqual(vm, key, entry->key)) {
            break;
        }

        index = (index + 1) & capacityMask;
        psl++;
    }

    dict->count--;

    for (;;) {
        DictItem *nextEntry;
        entry->key = EMPTY_VAL;
        entry->value = EMPTY_VAL;

        index = (index + 1) & capacityMask;
        nextEntry = &dict->entries[index];

        /*
         * Stop if we reach an empty bucket or hit a key which
         * is in its base (original) location.
         */
        if (IS_EMPTY(nextEntry->key) || nextEntry->psl == 0) {
            break;
        }

        nextEntry->psl--;
        *entry = *nextEntry;
        entry = nextEntry;
    }

    if (dict->count - 1 < dict->capacityMask * TABLE_MIN_LOAD) {
        // Figure out the new table size.
        capacityMask = SHRINK_CAPACITY(dict->capacityMask + 1) - 1;
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


static SetItem *findSetEntry(DictuVM *vm, SetItem *entries, int capacityMask,
                             Value value) {
    uint32_t index = hashValue(vm, value) & capacityMask;
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
        } else if (valuesEqual(vm, value, entry->value)) {
            // We found the key.
            return entry;
        }

        index = (index + 1) & capacityMask;
    }
}

bool setGet(DictuVM *vm, ObjSet *set, Value value) {
    if (set->count == 0) return false;

    SetItem *entry = findSetEntry(vm, set->entries, set->capacityMask, value);
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

        SetItem *dest = findSetEntry(vm, entries, capacityMask, entry->value);
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

    SetItem *entry = findSetEntry(vm, set->entries, set->capacityMask, value);
    bool isNewKey = IS_EMPTY(entry->value) || entry->deleted;
    entry->value = value;
    entry->deleted = false;

    if (isNewKey) set->count++;

    return isNewKey;
}

bool setDelete(DictuVM *vm, ObjSet *set, Value value) {
    if (set->count == 0) return false;

    SetItem *entry = findSetEntry(vm, set->entries, set->capacityMask, value);
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
char *valueToString(DictuVM *vm, Value value) {
    if (IS_BOOL(value)) {
        char *str = AS_BOOL(value) ? "true" : "false";
        char *boolString = ALLOCATE(vm, char, strlen(str) + 1);
        snprintf(boolString, strlen(str) + 1, "%s", str);
        return boolString;
    } else if (IS_NIL(value)) {
        char *nilString = ALLOCATE(vm, char, 4);
        snprintf(nilString, 4, "%s", "nil");
        return nilString;
    } else if (IS_NUMBER(value)) {
        double number = AS_NUMBER(value);
        int numberStringLength = snprintf(NULL, 0, "%.15g", number) + 1;
        char *numberString = ALLOCATE(vm, char, numberStringLength);
        snprintf(numberString, numberStringLength, "%.15g", number);
        return numberString;
    } else if (IS_OBJ(value)) {
        return objectToString(vm, value);
    }

    char *unknown = ALLOCATE(vm, char, 8);
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

void printValue(DictuVM *vm, Value value) {
    char *output = valueToString(vm, value);
    printf("%s", output);
    free(output);
}

void printValueError(DictuVM *vm, Value value) {
    char *output = valueToString(vm, value);
    fprintf(stderr, "%s", output);
    free(output);
}

static bool listComparison(DictuVM *vm, Value a, Value b) {
    ObjList *list = AS_LIST(a);
    ObjList *listB = AS_LIST(b);

    if (list->values.count != listB->values.count)
        return false;

    for (int i = 0; i < list->values.count; ++i) {
        if (!valuesEqual(vm, list->values.values[i], listB->values.values[i]))
            return false;
    }

    return true;
}

static bool dictComparison(DictuVM *vm, Value a, Value b) {
    ObjDict *dict = AS_DICT(a);
    ObjDict *dictB = AS_DICT(b);

    // Different lengths, not the same
    if (dict->count != dictB->count)
        return false;

    // Lengths are the same, and dict 1 has 0 length
    // therefore both are empty
    if (dict->count == 0)
        return true;

    for (int i = 0; i <= dict->capacityMask; ++i) {
        DictItem *item = &dict->entries[i];

        if (IS_EMPTY(item->key))
            continue;

        Value value;
        // Check if key from dict A is in dict B
        if (!dictGet(vm, dictB, item->key, &value)) {
            // Key doesn't exist
            return false;
        }

        // Key exists
        if (!valuesEqual(vm, item->value, value)) {
            // Values don't equal
            return false;
        }
    }

    return true;
}

static bool setComparison(DictuVM *vm, Value a, Value b) {
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
        if (!setGet(vm, setB, item->value)) {
            // Key doesn't exist
            return false;
        }
    }

    return true;
}

bool valuesEqual(DictuVM *vm, Value a, Value b) { 
    if (IS_OBJ(a) && IS_OBJ(b)) {
        if (AS_OBJ(a)->type != AS_OBJ(b)->type) return false;

        switch (AS_OBJ(a)->type) {
            case OBJ_LIST: {
                return listComparison(vm, a, b);
            }

            case OBJ_DICT: {
                return dictComparison(vm, a, b);
            }

            case OBJ_SET: {
                return setComparison(vm, a, b);
            }

                // Pass through
            default:
                break;
        }
    }

    return a == b;
}