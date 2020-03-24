#include "sets.h"

static bool addSetItem(VM *vm, int argCount) {
    if (argCount != 2) {
        runtimeError(vm, "add() takes 2 arguments (%d given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(vm, 0))) {
        runtimeError(vm, "Argument passed to add() must be a string", argCount);
        return false;
    }

    Value value = pop(vm);
    ObjSet *set = AS_SET(pop(vm));
    insertSet(vm, set, value);
    push(vm, NIL_VAL);

    return true;
}

static bool removeSetItem(VM *vm, int argCount) {
    if (argCount != 2) {
        runtimeError(vm, "remove() takes 2 arguments (%d  given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(vm, 0))) {
        runtimeError(vm, "Argument passed to remove() must be a string");
        return false;
    }

    ObjString *string = AS_STRING(pop(vm));
    ObjSet *set = AS_SET(pop(vm));
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
            resizeSet(set, false);
        }

        push(vm, NIL_VAL);
        return true;
    }

    runtimeError(vm, "Value '%s' passed to remove() does not exist within the set", string->chars);
    return false;
}

static bool containsSetItem(VM *vm, int argCount) {
    if (argCount != 2) {
        runtimeError(vm, "contains() takes 2 arguments (%d given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(vm, 0))) {
        runtimeError(vm, "Argument passed to contains() must be a string", argCount);
        return false;
    }

    ObjString *string = AS_STRING(pop(vm));
    ObjSet *set = AS_SET(pop(vm));

    if (searchSet(set, string)) {
        push(vm, TRUE_VAL);
    } else {
        push(vm, FALSE_VAL);
    }

    return true;
}

bool setMethods(VM *vm, char *method, int argCount) {
    if (strcmp(method, "add") == 0) {
        return addSetItem(vm, argCount);
    } else if (strcmp(method, "remove") == 0) {
        return removeSetItem(vm, argCount);
    } else if (strcmp(method, "contains") == 0) {
        return containsSetItem(vm, argCount);
    }

    runtimeError(vm, "Set has no method %s()", method);
    return false;
}
