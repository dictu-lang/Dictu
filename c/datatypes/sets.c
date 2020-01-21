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

bool setMethods(char *method, int argCount) {
    if (strcmp(method, "add") == 0) {
        return addSetItem(argCount);
    } else if (strcmp(method, "remove") == 0) {
        // TODO: Remove method for sets
    } else if (strcmp(method, "contains") == 0) {
        // TODO: Contains method for sets
    }

    runtimeError("Set has no method %s()", method);
    return false;
}
