#include "dicts.h"

static bool getDictItem(VM *vm, int argCount) {
    if (argCount != 2 && argCount != 3) {
        runtimeError(vm, "get() takes 2 or 3 arguments (%d  given)", argCount);
        return false;
    }

    Value defaultValue = NIL_VAL;
    if (argCount == 3) {
        defaultValue = pop(vm);
    }

    if (!IS_STRING(peek(vm, 0))) {
        runtimeError(vm, "Key passed to get() must be a string");
        return false;
    }

    Value key = pop(vm);
    ObjDict *dict = AS_DICT(pop(vm));

    Value ret = searchDict(dict, AS_CSTRING(key));

    if (ret == NIL_VAL) {
        push(vm, defaultValue);
    } else {
        push(vm, ret);
    }

    return true;
}

static bool removeDictItem(VM *vm, int argCount) {
    if (argCount != 2) {
        runtimeError(vm, "remove() takes 2 arguments (%d  given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(vm, 0))) {
        runtimeError(vm, "Key passed to remove() must be a string");
        return false;
    }

    char *key = AS_CSTRING(pop(vm));
    ObjDict *dict = AS_DICT(pop(vm));

    int index = hash(key) % dict->capacity;

    while (dict->items[index] && !(strcmp(dict->items[index]->key, key) == 0 && !dict->items[index]->deleted)) {
        index++;
        if (index == dict->capacity) {
            index = 0;
        }
    }

    if (dict->items[index]) {
        dict->items[index]->deleted = true;
        dict->count--;

        if (dict->capacity != 8 && dict->count * 100 / dict->capacity <= 35) {
            resizeDict(dict, false);
        }

        push(vm, NIL_VAL);
        return true;
    }

    runtimeError(vm, "Key '%s' passed to remove() does not exist within the dictionary", key);
    return false;
}

static bool dictItemExists(VM *vm, int argCount) {
    if (argCount != 2) {
        runtimeError(vm, "exists() takes 2 arguments (%d  given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(vm, 0))) {
        runtimeError(vm, "Key passed to exists() must be a string");
        return false;
    }

    char *key = AS_CSTRING(pop(vm));
    ObjDict *dict = AS_DICT(pop(vm));

    for (int i = 0; i < dict->capacity; ++i) {
        if (!dict->items[i])
            continue;

        if (strcmp(dict->items[i]->key, key) == 0 && !dict->items[i]->deleted) {
            push(vm, TRUE_VAL);
            return true;
        }
    }

    push(vm, FALSE_VAL);
    return true;
}

static bool copyDictShallow(VM *vm, int argCount) {
    if (argCount != 1) {
        runtimeError(vm, "copy() takes 1 argument (%d  given)", argCount);
        return false;
    }

    ObjDict *oldDict = AS_DICT(pop(vm));
    push(vm, OBJ_VAL(copyDict(vm, oldDict, true)));

    return true;
}

static bool copyDictDeep(VM *vm, int argCount) {
    if (argCount != 1) {
        runtimeError(vm, "deepCopy() takes 1 argument (%d  given)", argCount);
        return false;
    }

    ObjDict *oldDict = AS_DICT(pop(vm));
    push(vm, OBJ_VAL(copyDict(vm, oldDict, false)));

    return true;
}

bool dictMethods(VM *vm, char *method, int argCount) {
    if (strcmp(method, "get") == 0) {
        return getDictItem(vm, argCount);
    } else if (strcmp(method, "remove") == 0) {
        return removeDictItem(vm, argCount);
    } else if (strcmp(method, "exists") == 0) {
        return dictItemExists(vm, argCount);
    } else if (strcmp(method, "copy") == 0) {
        return copyDictShallow(vm, argCount);
    } else if (strcmp(method, "deepCopy") == 0) {
        return copyDictDeep(vm, argCount);
    }

    runtimeError(vm, "Dict has no method %s()", method);
    return false;
}
