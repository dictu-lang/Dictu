#include "copy.h"

uint32_t hash(char *str) {
    uint32_t hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}

ObjList *copyList(ObjList *oldList, bool shallow);

ObjDict *copyDict(ObjDict *oldDict, bool shallow) {
    // Ensure the GC does *not* run while we are creating the list
    vm.gc = false;

    ObjDict *newDict = initDict();

    for (int i = 0; i < oldDict->capacity; ++i) {
        if (oldDict->items[i] == NULL) {
            continue;
        }

        Value val = oldDict->items[i]->item;

        if (!shallow) {
            if (IS_DICT(val)) {
                val = OBJ_VAL(copyDict(AS_DICT(val), false));
            } else if (IS_LIST(val)) {
                val = OBJ_VAL(copyList(AS_LIST(val), false));
            }
        }

        ObjDict *dict = newDict;
        char *key = oldDict->items[i]->key;

        if (dict->count * 100 / dict->capacity >= 60) {
            resizeDict(dict, true);
        }

        uint32_t hashValue = hash(key);
        int index = hashValue % dict->capacity;

        size_t keySize = sizeof(char) * (strlen(key) + 1);
        char *key_m = realloc(NULL, keySize);
        vm.bytesAllocated += keySize;

        if (key_m == NULL) {
            printf("Unable to allocate memory\n");
            exit(71);
        }

        strcpy(key_m, key);
        size_t dictSize = sizeof(dictItem) * (sizeof(dictItem));
        dictItem *item = realloc(NULL, dictSize);
        vm.bytesAllocated += dictSize;

        if (item == NULL) {
            printf("Unable to allocate memory\n");
            exit(71);
        }

        item->key = key_m;
        item->item = val;
        item->deleted = false;
        item->hash = hashValue;

        while (dict->items[index] && !dict->items[index]->deleted && strcmp(dict->items[index]->key, key) != 0) {
            index++;
            if (index == dict->capacity) {
                index = 0;
            }
        }

        if (dict->items[index]) {
            free(dict->items[index]->key);
            free(dict->items[index]);
            dict->count--;
        }

        dict->items[index] = item;
        dict->count++;
    }

    vm.gc = true;
    return newDict;
}

ObjList *copyList(ObjList *oldList, bool shallow) {
    // Ensure the GC does *not* run while we are creating the list
    vm.gc = false;

    ObjList *newList = initList();

    for (int i = 0; i < oldList->values.count; ++i) {
        Value val = oldList->values.values[i];

        if (!shallow) {
            if (IS_DICT(val)) {
                val = OBJ_VAL(copyDict(AS_DICT(val), false));
            } else if (IS_LIST(val)) {
                val = OBJ_VAL(copyList(AS_LIST(val), false));
            }
        }

        ValueArray *array = &newList->values;

        if (array->capacity < array->count + 1) {
            int oldCapacity = array->capacity;
            array->capacity = GROW_CAPACITY(oldCapacity);
            vm.bytesAllocated += sizeof(Value) * (array->capacity) - sizeof(Value) * (oldCapacity);
            array->values = realloc(array->values, sizeof(Value) * (array->capacity));
        }

        array->values[array->count] = val;
        array->count++;
    }

    vm.gc = true;
    return newList;
}

// TODO: Set copy
