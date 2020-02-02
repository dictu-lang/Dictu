#include "instance.h"
#include "../vm.h"
#include "../memory.h"

static bool hasAttribute(int argCount) {
    if (argCount != 2) {
        runtimeError("hasAttribute() takes 2 arguments (%d  given)", argCount);
        return false;
    }

    Value value = pop(); // Pop the "attribute"
    ObjInstance *instance = AS_INSTANCE(pop()); // Pop the instance

    if (!IS_STRING(value)) {
        runtimeError("Argument passed to hasAttribute() must be a string");
        return false;
    }

    Value _; // Unused variable
    if (tableGet(&instance->fields, AS_STRING(value), &_)) {
        push(TRUE_VAL);
    } else {
        push(FALSE_VAL);
    }

    return true;
}

static bool getAttribute(int argCount) {
    if (argCount != 2 && argCount != 3) {
        runtimeError("getAttribute() takes 2 or 3 arguments (%d  given)", argCount);
        return false;
    }

    Value defaultValue = NIL_VAL;
    // Passed in a default value
    if (argCount == 3) {
        defaultValue = pop();
    }

    Value key = pop();

    if (!IS_STRING(key)) {
        runtimeError("Argument passed to getAttribute() must be a string");
        return false;
    }

    ObjInstance *instance = AS_INSTANCE(pop()); // Pop the instance

    Value value;
    if (tableGet(&instance->fields, AS_STRING(key), &value)) {
        push(value);
    } else {
        push(defaultValue);
    }

    return true;
}

static bool setAttribute(int argCount) {
    if (argCount != 3) {
        runtimeError("setAttribute() takes 3 (%d  given)", argCount);
        return false;
    }

    Value value = pop();
    Value key = pop();

    if (!IS_STRING(key)) {
        runtimeError("Argument passed to setAttribute() must be a string");
        return false;
    }

    ObjInstance *instance = AS_INSTANCE(pop()); // Pop the instance

    tableSet(&instance->fields, AS_STRING(key), value);

    push(NIL_VAL);

    return true;
}


bool instanceMethods(char *method, int argCount) {
    if (strcmp(method, "hasAttribute") == 0) {
        return hasAttribute(argCount);
    } else if (strcmp(method, "getAttribute") == 0) {
        return getAttribute(argCount);
    } else if (strcmp(method, "setAttribute") == 0) {
        return setAttribute(argCount);
    }

    return false;
}
