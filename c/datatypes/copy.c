#include "copy.h"

uint32_t hash(char *str) {
    uint32_t hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}

ObjList *copyList(VM* vm, ObjList *oldList, bool shallow);

ObjDict *copyDict(VM* vm, ObjDict *oldDict, bool shallow) {
    ObjDict *newDict = initDict(vm);
    // Push to stack to avoid GC
    push(vm, OBJ_VAL(newDict));

    for (int i = 0; i < oldDict->capacity; ++i) {
        if (oldDict->items[i] == NULL) {
            continue;
        }

        Value val = oldDict->items[i]->item;

        if (!shallow) {
            if (IS_DICT(val)) {
                val = OBJ_VAL(copyDict(vm, AS_DICT(val), false));
            } else if (IS_LIST(val)) {
                val = OBJ_VAL(copyList(vm, AS_LIST(val), false));
            } else if (IS_INSTANCE(val)) {
                val = OBJ_VAL(copyInstance(vm, AS_INSTANCE(val), false));
            }
        }

        // Push to stack to avoid GC
        push(vm, val);
        insertDict(vm, newDict, oldDict->items[i]->key, val);
        pop(vm);
    }

    pop(vm);
    return newDict;
}

ObjList *copyList(VM* vm, ObjList *oldList, bool shallow) {
    ObjList *newList = initList(vm);
    // Push to stack to avoid GC
    push(vm, OBJ_VAL(newList));

    for (int i = 0; i < oldList->values.count; ++i) {
        Value val = oldList->values.values[i];

        if (!shallow) {
            if (IS_DICT(val)) {
                val = OBJ_VAL(copyDict(vm, AS_DICT(val), false));
            } else if (IS_LIST(val)) {
                val = OBJ_VAL(copyList(vm, AS_LIST(val), false));
            } else if (IS_INSTANCE(val)) {
                val = OBJ_VAL(copyInstance(vm, AS_INSTANCE(val), false));
            }
        }

        // Push to stack to avoid GC
        push(vm, val);
        writeValueArray(vm, &newList->values, val);
        pop(vm);
    }

    pop(vm);
    return newList;
}

ObjInstance *copyInstance(VM* vm, ObjInstance *oldInstance, bool shallow) {
    ObjInstance *instance = newInstance(vm, oldInstance->klass);
    // Push to stack to avoid GC
    push(vm, OBJ_VAL(instance));

    if (shallow) {
        tableAddAll(vm, &oldInstance->fields, &instance->fields);
    } else {
        for (int i = 0; i <= oldInstance->fields.capacityMask; i++) {
            Entry *entry = &oldInstance->fields.entries[i];
            if (entry->key != NULL) {
                Value val = entry->value;

                if (IS_LIST(val)) {
                    val = OBJ_VAL(copyList(vm, AS_LIST(val), false));
                } else if (IS_DICT(val)) {
                    val = OBJ_VAL(copyDict(vm, AS_DICT(val), false));
                } else if (IS_INSTANCE(val)) {
                    val = OBJ_VAL(copyInstance(vm, AS_INSTANCE(val), false));
                }

                // Push to stack to avoid GC
                push(vm, val);
                tableSet(vm, &instance->fields, entry->key, val);
                pop(vm);
            }
        }
    }

    pop(vm);
    return instance;
}

// TODO: Set copy
