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

    Value ret;
    if (tableGet(&dict->items, AS_STRING(key), &ret)) {
        push(vm, ret);
    } else {
        push(vm, defaultValue);
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

    ObjString *key = AS_STRING(pop(vm));
    ObjDict *dict = AS_DICT(pop(vm));

    if (tableDelete(&dict->items, key)) {
        push(vm, NIL_VAL);
        return true;
    }

    runtimeError(vm, "Key '%s' passed to remove() does not exist within the dictionary", key->chars);
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

    ObjString *key = AS_STRING(pop(vm));
    ObjDict *dict = AS_DICT(pop(vm));
    Value v;

    if (tableGet(&dict->items, key, &v)) {
        push(vm, TRUE_VAL);
    } else {
        push(vm, FALSE_VAL);
    }

    return true;
}

static bool copyDictShallow(VM *vm, int argCount) {
    if (argCount != 1) {
        runtimeError(vm, "copy() takes 1 argument (%d  given)", argCount);
        return false;
    }

    ObjDict *oldDict = AS_DICT(peek(vm, 0));
    ObjDict *newDict = copyDict(vm, oldDict, true);
    pop(vm);
    push(vm, OBJ_VAL(newDict));

    return true;
}

static bool copyDictDeep(VM *vm, int argCount) {
    if (argCount != 1) {
        runtimeError(vm, "deepCopy() takes 1 argument (%d  given)", argCount);
        return false;
    }

    ObjDict *oldDict = AS_DICT(peek(vm, 0));
    ObjDict *newDict = copyDict(vm, oldDict, false);
    pop(vm);
    push(vm, OBJ_VAL(newDict));

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
