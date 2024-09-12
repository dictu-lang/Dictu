#include "copy.h"

ObjList *copyList(DictuVM* vm, ObjList *oldList, bool shallow);

ObjDict *copyDict(DictuVM* vm, ObjDict *oldDict, bool shallow) {
    ObjDict *dict = newDict(vm);
    // Push to stack to avoid GC
    push(vm, OBJ_VAL(dict));

    for (int i = 0; i <= oldDict->capacityMask; ++i) {
        if (IS_EMPTY(oldDict->entries[i].key)) {
            continue;
        }

        Value val = oldDict->entries[i].value;

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
        dictSet(vm, dict, oldDict->entries[i].key, val);
        pop(vm);
    }

    pop(vm);
    return dict;
}

ObjList *copyList(DictuVM* vm, ObjList *oldList, bool shallow) {
    ObjList *list = newList(vm);
    // Push to stack to avoid GC
    push(vm, OBJ_VAL(list));

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
        writeValueArray(vm, &list->values, val);
        pop(vm);
    }

    pop(vm);
    return list;
}

ObjInstance *copyInstance(DictuVM* vm, ObjInstance *oldInstance, bool shallow) {
    ObjInstance *instance = newInstance(vm, oldInstance->klass);
    // Push to stack to avoid GC
    push(vm, OBJ_VAL(instance));

    if (shallow) {
        tableAddAll(vm, &oldInstance->publicAttributes, &instance->publicAttributes);
    } else {
        for (int i = 0; i < oldInstance->publicAttributes.capacity; i++) {
            Entry *entry = &oldInstance->publicAttributes.entries[i];
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
                tableSet(vm, &instance->publicAttributes, entry->key, val);
                pop(vm);
            }
        }

        for (int i = 0; i < oldInstance->privateAttributes.capacity; i++) {
            Entry *entry = &oldInstance->privateAttributes.entries[i];
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
                tableSet(vm, &instance->privateAttributes, entry->key, val);
                pop(vm);
            }
        }
    }

    pop(vm);
    return instance;
}

// TODO: Set copy
