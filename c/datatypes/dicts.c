#include "dicts.h"

static bool getDictItem(int argCount) {
    if (argCount != 2 && argCount != 3) {
        runtimeError("get() takes 2 or 3 arguments (%d  given)", argCount);
        return false;
    }

    Value defaultValue = NIL_VAL;
    if (argCount == 3) {
        defaultValue = pop();
    }

    if (!IS_STRING(peek(0))) {
        runtimeError("Key passed to get() must be a string");
        return false;
    }

    Value key = pop();
    ObjDict *dict = AS_DICT(pop());

    Value ret = searchDict(dict, AS_CSTRING(key));

    if (ret == NIL_VAL) {
        push(defaultValue);
    } else {
        push(ret);
    }

    return true;
}

static bool removeDictItem(int argCount) {
    if (argCount != 2) {
        runtimeError("remove() takes 2 arguments (%d  given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(0))) {
        runtimeError("Key passed to remove() must be a string");
        return false;
    }

    char *key = AS_CSTRING(pop());
    ObjDict *dict = AS_DICT(pop());

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

        push(NIL_VAL);
        return true;
    }

    runtimeError("Key '%s' passed to remove() does not exist within the dictionary", key);
    return false;
}

static bool dictItemExists(int argCount) {
    if (argCount != 2) {
        runtimeError("exists() takes 2 arguments (%d  given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(0))) {
        runtimeError("Key passed to exists() must be a string");
        return false;
    }

    char *key = AS_CSTRING(pop());
    ObjDict *dict = AS_DICT(pop());

    for (int i = 0; i < dict->capacity; ++i) {
        if (!dict->items[i])
            continue;

        if (strcmp(dict->items[i]->key, key) == 0 && !dict->items[i]->deleted) {
            push(TRUE_VAL);
            return true;
        }
    }

    push(FALSE_VAL);
    return true;
}

static bool copyDictShallow(int argCount) {
    if (argCount != 1) {
        runtimeError("copy() takes 1 argument (%d  given)", argCount);
        return false;
    }

    ObjDict *oldDict = AS_DICT(pop());
    push(OBJ_VAL(copyDict(oldDict, true)));

    return true;
}

static bool copyDictDeep(int argCount) {
    if (argCount != 1) {
        runtimeError("deepCopy() takes 1 argument (%d  given)", argCount);
        return false;
    }

    ObjDict *oldDict = AS_DICT(pop());
    push(OBJ_VAL(copyDict(oldDict, false)));

    return true;
}

bool dictMethods(char *method, int argCount) {
    if (strcmp(method, "get") == 0) {
        return getDictItem(argCount);
    } else if (strcmp(method, "remove") == 0) {
        return removeDictItem(argCount);
    } else if (strcmp(method, "exists") == 0) {
        return dictItemExists(argCount);
    } else if (strcmp(method, "copy") == 0) {
        return copyDictShallow(argCount);
    } else if (strcmp(method, "deepCopy") == 0) {
        return copyDictDeep(argCount);
    }

    runtimeError("Dict has no method %s()", method);
    return false;
}
