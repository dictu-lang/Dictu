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
    ObjDict *newDict = initDict();
    // Push to stack to avoid GC
    push(OBJ_VAL(newDict));

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
            } else if (IS_INSTANCE(val)) {
                val = OBJ_VAL(copyInstance(AS_INSTANCE(val), false));
            }
        }

        // Push to stack to avoid GC
        push(val);
        insertDict(newDict, oldDict->items[i]->key, val);
        pop();
    }

    pop();
    return newDict;
}

ObjList *copyList(ObjList *oldList, bool shallow) {
    ObjList *newList = initList();
    // Push to stack to avoid GC
    push(OBJ_VAL(newList));

    for (int i = 0; i < oldList->values.count; ++i) {
        Value val = oldList->values.values[i];

        if (!shallow) {
            if (IS_DICT(val)) {
                val = OBJ_VAL(copyDict(AS_DICT(val), false));
            } else if (IS_LIST(val)) {
                val = OBJ_VAL(copyList(AS_LIST(val), false));
            } else if (IS_INSTANCE(val)) {
                val = OBJ_VAL(copyInstance(AS_INSTANCE(val), false));
            }
        }

        // Push to stack to avoid GC
        push(val);
        writeValueArray(&newList->values, val);
        pop();
    }

    pop();
    return newList;
}

ObjInstance *copyInstance(ObjInstance *oldInstance, bool shallow) {
    ObjInstance *instance = newInstance(oldInstance->klass);
    // Push to stack to avoid GC
    push(OBJ_VAL(instance));

    if (shallow) {
        tableAddAll(&oldInstance->fields, &instance->fields);
    } else {
        for (int i = 0; i <= oldInstance->fields.capacityMask; i++) {
            Entry *entry = &oldInstance->fields.entries[i];
            if (entry->key != NULL) {
                Value val = entry->value;

                if (IS_LIST(val)) {
                    val = OBJ_VAL(copyList(AS_LIST(val), false));
                } else if (IS_DICT(val)) {
                    val = OBJ_VAL(copyDict(AS_DICT(val), false));
                } else if (IS_INSTANCE(val)) {
                    val = OBJ_VAL(copyInstance(AS_INSTANCE(val), false));
                }

                // Push to stack to avoid GC
                push(val);
                tableSet(&instance->fields, entry->key, val);
                pop();
            }
        }
    }

    pop();
    return instance;
}

// TODO: Set copy
