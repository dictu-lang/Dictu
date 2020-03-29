#include "dicts.h"

static Value getDictItem(VM *vm, int argCount, Value *args) {
    if (argCount != 1 && argCount != 2) {
        runtimeError(vm, "get() takes 1 or 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Value defaultValue = NIL_VAL;
    if (argCount == 2) {
        defaultValue = args[2];
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "Key passed to get() must be a string");
        return EMPTY_VAL;
    }

    ObjDict *dict = AS_DICT(args[0]);

    Value ret;
    if (dictGet(dict, args[1], &ret)) {
        return ret;
    }

    return defaultValue;
}

static Value removeDictItem(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "remove() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "Key passed to remove() must be a string");
        return EMPTY_VAL;
    }

    ObjDict *dict = AS_DICT(args[0]);

    if (dictDelete(dict, args[1])) {
        return NIL_VAL;
    }

    runtimeError(vm, "Key '%s' passed to remove() does not exist within the dictionary", AS_CSTRING(args[1]));
    return EMPTY_VAL;
}

static Value dictItemExists(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "exists() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "Key passed to exists() must be a string");
        return EMPTY_VAL;
    }

    ObjDict *dict = AS_DICT(args[0]);

    if (dict->count == 0) {
        return FALSE_VAL;
    }

    Value v;
    if (dictGet(dict, args[1], &v)) {
        return TRUE_VAL;
    }

    return FALSE_VAL;
}

static Value copyDictShallow(VM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "copy() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjDict *oldDict = AS_DICT(args[0]);
    ObjDict *newDict = copyDict(vm, oldDict, true);

    return OBJ_VAL(newDict);
}

static Value copyDictDeep(VM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "deepCopy() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjDict *oldDict = AS_DICT(args[0]);
    ObjDict *newDict = copyDict(vm, oldDict, false);

    return OBJ_VAL(newDict);
}

void declareDictMethods(VM *vm) {
    defineNative(vm, &vm->dictMethods, "get", getDictItem);
    defineNative(vm, &vm->dictMethods, "remove", removeDictItem);
    defineNative(vm, &vm->dictMethods, "exists", dictItemExists);
    defineNative(vm, &vm->dictMethods, "copy", copyDictShallow);
    defineNative(vm, &vm->dictMethods, "deepCopy", copyDictDeep);
}