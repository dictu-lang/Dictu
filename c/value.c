#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "memory.h"
#include "value.h"
#include "vm.h"

#define TABLE_MAX_LOAD 0.75
#define TABLE_MIN_LOAD 0.35

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

static inline uint32_t hashBits(uint64_t hash)
{
    // From v8's ComputeLongHash() which in turn cites:
    // Thomas Wang, Integer Hash Functions.
    // http://www.concentric.net/~Ttwang/tech/inthash.htm
    hash = ~hash + (hash << 18);  // hash = (hash << 18) - hash - 1;
    hash = hash ^ (hash >> 31);
    hash = hash * 21;  // hash = (hash + (hash << 2)) + (hash << 4);
    hash = hash ^ (hash >> 11);
    hash = hash + (hash << 6);
    hash = hash ^ (hash >> 22);
    return (uint32_t)(hash & 0x3fffffff);
}

static uint32_t hashObject(Obj* object) {
    switch (object->type) {
        case OBJ_STRING: {
            return ((ObjString*)object)->hash;
        }

        // Should never get here
        default: {
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

void freeDict(VM *vm, ObjDict *dict) {
    FREE_ARRAY(vm, DictItem, dict->entries, dict->capacityMask + 1);
    FREE(vm, ObjDict, dict);
}

static DictItem *findEntry(DictItem *entries, int capacityMask,
                           Value key) {
    uint32_t index = hashValue(key) & capacityMask;
    DictItem *tombstone = NULL;

    for (;;) {
        // printf("lol\n");
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

        index = (index + 1) & capacityMask;
    }
}

bool dictGet(ObjDict *dict, Value key, Value *value) {
    if (dict->entries == NULL) return false;

    DictItem *entry = findEntry(dict->entries, dict->capacityMask, key);
    if (IS_EMPTY(entry->key)) return false;

    *value = entry->value;
    return true;
}

static void adjustCapacity(VM *vm, ObjDict *dict, int capacityMask) {
    DictItem *entries = ALLOCATE(vm, DictItem, capacityMask + 1);
    for (int i = 0; i <= capacityMask; i++) {
        entries[i].key = EMPTY_VAL;
        entries[i].value = NIL_VAL;
    }

    // printf("Mask: %d\nCount: %d\n", dict->capacityMask, dict->count);

    dict->count = 0;

    for (int i = 0; i <= dict->capacityMask; i++) {
        DictItem *entry = &dict->entries[i];
        if (IS_EMPTY(entry->key)) continue;

        DictItem *dest = findEntry(entries, capacityMask, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        dict->count++;
    }

    FREE_ARRAY(vm, DictItem, dict->entries, dict->capacityMask + 1);
    dict->entries = entries;
    dict->capacityMask = capacityMask;
}

bool dictSet(VM *vm, ObjDict *dict, Value key, Value value) {
    // printf("Size: %d\nMask: %d\n", (dict->count + 1) / dict->capacityMask, dict->capacityMask);
    if (dict->count + 1 > (dict->capacityMask + 1) * TABLE_MAX_LOAD) {
        // Figure out the new table size.
        int capacityMask = GROW_CAPACITY(dict->capacityMask + 1) - 1;
        adjustCapacity(vm, dict, capacityMask);
    }

    DictItem *entry = findEntry(dict->entries, dict->capacityMask, key);
    bool isNewKey = IS_EMPTY(entry->key);
    entry->key = key;
    entry->value = value;

    if (isNewKey) dict->count++;
    return isNewKey;
}

bool dictDelete(VM *vm, ObjDict *dict, Value key) {
    if (dict->count == 0) return false;

    DictItem *entry = findEntry(dict->entries, dict->capacityMask, key);
    if (IS_EMPTY(entry->key)) return false;

    // Place a tombstone in the entry.
    dict->count--;
    entry->key = EMPTY_VAL;
    entry->value = BOOL_VAL(true);

    if (dict->count - 1 < dict->capacityMask * TABLE_MIN_LOAD) {
        // Figure out the new table size.
        int capacityMask = SHRINK_CAPACITY(dict->capacityMask);
        adjustCapacity(vm, dict, capacityMask);
    }

    return true;
}

void grayDict(VM *vm, ObjDict *dict) {
    for (int i = 0; i <= dict->capacityMask; i++) {
        DictItem *entry = &dict->entries[i];
        grayValue(vm, entry->key);
        grayValue(vm, entry->value);
    }
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