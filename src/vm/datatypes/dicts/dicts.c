#include "dicts.h"

/*
 * Note: We should try to implement everything we can in C
 *       rather than in the host language as C will always
 *       be faster than Dictu, and there will be extra work
 *       at startup running the Dictu code, however compromises
 *       must be made due to the fact the VM is not re-enterable
 */

#include "dict-source.h"

static Value toStringDict(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "toString() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    char *valueString = dictToString(vm, args[0]);

    ObjString *string = copyString(vm, valueString, strlen(valueString));
    free(valueString);

    return OBJ_VAL(string);
}

static Value lenDict(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "len() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjDict *dict = AS_DICT(args[0]);
    return NUMBER_VAL(dict->count);
}

static Value keysDict(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "keys() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjDict *dict = AS_DICT(args[0]);

    ObjList *list = newList(vm);
    push(vm, OBJ_VAL(list));

    for (int i = 0; i < dict->capacityMask + 1; ++i) {
        if (IS_EMPTY(dict->entries[i].key)) {
            continue;
        }

        writeValueArray(vm, &list->values, dict->entries[i].key);
    }

    pop(vm);
    return OBJ_VAL(list);
}

static Value getDictItem(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1 && argCount != 2) {
        runtimeError(vm, "get() takes 1 or 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Value defaultValue = NIL_VAL;
    if (argCount == 2) {
        defaultValue = args[2];
    }

    if (!isValidKey(args[1])) {
        runtimeError(vm, "Dictionary key passed to get() must be an immutable type");
        return EMPTY_VAL;
    }

    ObjDict *dict = AS_DICT(args[0]);

    Value ret;
    if (dictGet(vm, dict, args[1], &ret)) {
        return ret;
    }

    return defaultValue;
}

static Value removeDictItem(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "remove() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!isValidKey(args[1])) {
        runtimeError(vm, "Dictionary key passed to remove() must be an immutable type");
        return EMPTY_VAL;
    }

    ObjDict *dict = AS_DICT(args[0]);

    if (dictDelete(vm, dict, args[1])) {
        return NIL_VAL;
    }

    char *str = valueToString(vm, args[1]);
    runtimeError(vm, "Key '%s' passed to remove() does not exist within the dictionary", str);
    FREE(vm, char, str);

    return EMPTY_VAL;
}

static Value dictItemExists(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "exists() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!isValidKey(args[1])) {
        runtimeError(vm, "Dictionary key passed to exists() must be an immutable type");
        return EMPTY_VAL;
    }

    ObjDict *dict = AS_DICT(args[0]);

    if (dict->count == 0) {
        return FALSE_VAL;
    }

    Value v;
    if (dictGet(vm, dict, args[1], &v)) {
        return TRUE_VAL;
    }

    return FALSE_VAL;
}

static Value copyDictShallow(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "copy() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjDict *oldDict = AS_DICT(args[0]);
    ObjDict *newDict = copyDict(vm, oldDict, true);

    return OBJ_VAL(newDict);
}

static Value copyDictDeep(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "deepCopy() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjDict *oldDict = AS_DICT(args[0]);
    ObjDict *newDict = copyDict(vm, oldDict, false);

    return OBJ_VAL(newDict);
}

void declareDictMethods(DictuVM *vm) {
    defineNative(vm, &vm->dictMethods, "toString", toStringDict);
    defineNative(vm, &vm->dictMethods, "len", lenDict);
    defineNative(vm, &vm->dictMethods, "keys", keysDict);
    defineNative(vm, &vm->dictMethods, "get", getDictItem);
    defineNative(vm, &vm->dictMethods, "remove", removeDictItem);
    defineNative(vm, &vm->dictMethods, "exists", dictItemExists);
    defineNative(vm, &vm->dictMethods, "copy", copyDictShallow);
    defineNative(vm, &vm->dictMethods, "deepCopy", copyDictDeep);
    defineNative(vm, &vm->dictMethods, "toBool", boolNative); // Defined in util

    dictuInterpret(vm, "Dict", DICTU_DICT_SOURCE);

    Value Dict;
    tableGet(&vm->modules, copyString(vm, "Dict", 4), &Dict);

    ObjModule *DictModule = AS_MODULE(Dict);
    push(vm, Dict);
    tableAddAll(vm, &DictModule->values, &vm->dictMethods);
    pop(vm);
}
