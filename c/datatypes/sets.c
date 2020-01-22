#include "sets.h"

static bool addSetItem(int argCount) {
    if (argCount != 2) {
        runtimeError("add() takes 2 arguments (%d given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(0))) {
        runtimeError("Argument passed to add() must be a string", argCount);
        return false;
    }

    Value value = pop();
    ObjSet *set = AS_SET(pop());
    insertSet(set, value);
    push(NIL_VAL);

    return true;
}

static bool removeSetItem(int argCount) {
    if (argCount != 2) {
        runtimeError("remove() takes 2 arguments (%d  given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(0))) {
        runtimeError("Argument passed to remove() must be a string");
        return false;
    }

    ObjString *string = AS_STRING(pop());
    ObjSet *set = AS_SET(pop());
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

        push(NIL_VAL);
        return true;
    }

    runtimeError("Value '%s' passed to remove() does not exist within the set", string->chars);
    return false;
}

static bool containsSetItem(int argCount) {
    if (argCount != 2) {
        runtimeError("contains() takes 2 arguments (%d given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(0))) {
        runtimeError("Argument passed to contains() must be a string", argCount);
        return false;
    }

    ObjString *string = AS_STRING(pop());
    ObjSet *set = AS_SET(pop());

    if (searchSet(set, string)) {
        push(TRUE_VAL);
    } else {
        push(FALSE_VAL);
    }

    return true;
}

bool setMethods(char *method, int argCount) {
    if (strcmp(method, "add") == 0) {
        return addSetItem(argCount);
    } else if (strcmp(method, "remove") == 0) {
        return removeSetItem(argCount);
    } else if (strcmp(method, "contains") == 0) {
        return containsSetItem(argCount);
    }

    runtimeError("Set has no method %s()", method);
    return false;
}
