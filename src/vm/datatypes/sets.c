#include "sets.h"

static Value toStringSet(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "toString() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    char *valueString = setToString(vm, args[0]);

    ObjString *string = copyString(vm, valueString, strlen(valueString));
    free(valueString);

    return OBJ_VAL(string);
}

static Value lenSet(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "len() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjSet *set = AS_SET(args[0]);
    return NUMBER_VAL(set->count);
}

static Value addSetItem(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "add() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!isValidKey(args[1])) {
        runtimeError(vm, "Set value must be an immutable type");
        return EMPTY_VAL;
    }

    ObjSet *set = AS_SET(args[0]);
    setInsert(vm, set, args[1]);

    return NIL_VAL;
}

static Value removeSetItem(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "remove() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjSet *set = AS_SET(args[0]);

    if (!setDelete(vm, set, args[1])) {
        char *str = valueToString(vm, args[1]);
        runtimeError(vm, "Value '%s' passed to remove() does not exist within the set", str);
        FREE(vm, char, str);
        return EMPTY_VAL;
    }

    return NIL_VAL;
}

static Value containsSetItem(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "contains() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjSet *set = AS_SET(args[0]);

    return setGet(vm, set, args[1]) ? TRUE_VAL : FALSE_VAL;
}

static Value containsAllSet(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "containsAll() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_LIST(args[1])){
        runtimeError(vm, "containsAll() argument must be a list");
        return EMPTY_VAL;
    }

    ObjSet *set = AS_SET(args[0]);
    ObjList *list = AS_LIST(args[1]);

    int listSize = list->values.count;
    for (int index=0;index<listSize;index++){
        if (setGet(vm, set, list->values.values[index])==false){
            return FALSE_VAL;
        }
    }
    return TRUE_VAL;
}

void declareSetMethods(DictuVM *vm) {
    defineNative(vm, &vm->setMethods, "toString", toStringSet);
    defineNative(vm, &vm->setMethods, "len", lenSet);
    defineNative(vm, &vm->setMethods, "add", addSetItem);
    defineNative(vm, &vm->setMethods, "remove", removeSetItem);
    defineNative(vm, &vm->setMethods, "contains", containsSetItem);
    defineNative(vm, &vm->setMethods, "containsAll", containsAllSet);
    defineNative(vm, &vm->setMethods, "toBool", boolNative); // Defined in util
}

