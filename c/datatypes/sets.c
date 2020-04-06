#include "sets.h"


static Value lenSet(VM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "len() takes 0 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjSet *set = AS_SET(args[0]);
    return NUMBER_VAL(set->count);
}

static Value addSetItem(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "add() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "Argument passed to add() must be a string", argCount);
        return EMPTY_VAL;
    }


    ObjSet *set = AS_SET(args[0]);
    insertSet(vm, set, args[1]);

    return NIL_VAL;
}

static Value removeSetItem(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "remove() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "Argument passed to remove() must be a string");
        return EMPTY_VAL;
    }

    ObjSet *set = AS_SET(args[0]);
    ObjString *string = AS_STRING(args[1]);
    int index = string->hash % set->capacity;

    while (set->items[index] && !(strcmp(set->items[index]->item->chars, string->chars) == 0 && !set->items[index]->deleted)) {
        index++;
        if (index == set->capacity) {
            index = 0;
        }
    }

    if (set->items[index]) {
        set->items[index]->deleted = true;
        set->count--;

        if (set->capacity != 8 && set->count * 100 / set->capacity <= 35) {
            resizeSet(vm, set, false);
        }

        return NIL_VAL;
    }

    runtimeError(vm, "Value '%s' passed to remove() does not exist within the set", string->chars);
    return EMPTY_VAL;
}

static Value containsSetItem(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "contains() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "Argument passed to contains() must be a string", argCount);
        return EMPTY_VAL;
    }

    ObjSet *set = AS_SET(args[0]);
    ObjString *string = AS_STRING(args[1]);

    if (searchSet(set, string)) {
        return TRUE_VAL;
    }

    return FALSE_VAL;
}

void declareSetMethods(VM *vm) {
    defineNative(vm, &vm->setMethods, "len", lenSet);
    defineNative(vm, &vm->setMethods, "add", addSetItem);
    defineNative(vm, &vm->setMethods, "remove", removeSetItem);
    defineNative(vm, &vm->setMethods, "contains", containsSetItem);
}

