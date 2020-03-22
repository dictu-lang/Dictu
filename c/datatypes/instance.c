#include "instance.h"
#include "../vm.h"
#include "../memory.h"

static bool hasAttribute(VM *vm, int argCount) {
    if (argCount != 2) {
        runtimeError(vm, "hasAttribute() takes 2 arguments (%d  given)", argCount);
        return false;
    }

    Value value = pop(vm); // Pop the "attribute"
    ObjInstance *instance = AS_INSTANCE(pop(vm)); // Pop the instance

    if (!IS_STRING(value)) {
        runtimeError(vm, "Argument passed to hasAttribute() must be a string");
        return false;
    }

    Value _; // Unused variable
    if (tableGet(&instance->fields, AS_STRING(value), &_)) {
        push(vm, TRUE_VAL);
    } else {
        push(vm, FALSE_VAL);
    }

    return true;
}

static bool getAttribute(VM *vm, int argCount) {
    if (argCount != 2 && argCount != 3) {
        runtimeError(vm, "getAttribute() takes 2 or 3 arguments (%d  given)", argCount);
        return false;
    }

    Value defaultValue = NIL_VAL;
    // Passed in a default value
    if (argCount == 3) {
        defaultValue = pop(vm);
    }

    Value key = pop(vm);

    if (!IS_STRING(key)) {
        runtimeError(vm, "Argument passed to getAttribute() must be a string");
        return false;
    }

    ObjInstance *instance = AS_INSTANCE(pop(vm)); // Pop the instance

    Value value;
    if (tableGet(&instance->fields, AS_STRING(key), &value)) {
        push(vm, value);
    } else {
        push(vm, defaultValue);
    }

    return true;
}

static bool setAttribute(VM *vm, int argCount) {
    if (argCount != 3) {
        runtimeError(vm, "setAttribute() takes 3 arguments (%d  given)", argCount);
        return false;
    }

    Value value = pop(vm);
    Value key = pop(vm);

    if (!IS_STRING(key)) {
        runtimeError(vm, "Argument passed to setAttribute() must be a string");
        return false;
    }

    ObjInstance *instance = AS_INSTANCE(pop(vm)); // Pop the instance

    tableSet(vm, &instance->fields, AS_STRING(key), value);

    push(vm, NIL_VAL);

    return true;
}

static bool copyShallow(VM *vm, int argCount) {
    if (argCount != 1) {
        runtimeError(vm, "copy() takes 1 argument (%d  given)", argCount);
        return false;
    }

    ObjInstance *oldInstance = AS_INSTANCE(pop(vm));
    ObjInstance *instance = copyInstance(vm, oldInstance, true);
    push(vm, OBJ_VAL(instance));

    return true;
}

static bool copyDeep(VM *vm, int argCount) {
    if (argCount != 1) {
        runtimeError(vm, "deepCopy() takes 1 argument (%d  given)", argCount);
        return false;
    }

    ObjInstance *oldInstance = AS_INSTANCE(pop(vm));
    ObjInstance *instance = copyInstance(vm, oldInstance, false);
    push(vm, OBJ_VAL(instance));

    return true;
}

bool instanceMethods(VM *vm, char *method, int argCount) {
    if (strcmp(method, "hasAttribute") == 0) {
        return hasAttribute(vm, argCount);
    } else if (strcmp(method, "getAttribute") == 0) {
        return getAttribute(vm, argCount);
    } else if (strcmp(method, "setAttribute") == 0) {
        return setAttribute(vm, argCount);
    } else if (strcmp(method, "copy") == 0) {
        return copyShallow(vm, argCount);
    } else if (strcmp(method, "deepCopy") == 0) {
        return copyDeep(vm, argCount);
    }

    return false;
}
