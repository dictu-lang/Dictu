#include "instance.h"
#include "../vm.h"
#include "../memory.h"

static Value toString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "toString() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    char *valueString = instanceToString(args[0]);

    ObjString *string = copyString(vm, valueString, strlen(valueString));
    free(valueString);

    return OBJ_VAL(string);
}


static Value hasAttribute(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "hasAttribute() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjInstance *instance = AS_INSTANCE(args[0]);
    Value value = args[1];

    if (!IS_STRING(value)) {
        runtimeError(vm, "Argument passed to hasAttribute() must be a string");
        return EMPTY_VAL;
    }

    Value _; // Unused variable
    if (tableGet(&instance->publicFields, AS_STRING(value), &_)) {
        return TRUE_VAL;
    }

    if (tableGet(&instance->klass->publicMethods, AS_STRING(value), &value)) {
        return TRUE_VAL;
    }

    // Check class for properties
    ObjClass *klass = instance->klass;

    while (klass != NULL) {
        if (tableGet(&klass->publicProperties, AS_STRING(value), &value)) {
            return TRUE_VAL;
        }

        klass = klass->superclass;
    }

    return FALSE_VAL;
}

static Value getAttribute(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1 && argCount != 2) {
        runtimeError(vm, "getAttribute() takes 1 or 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Value defaultValue = NIL_VAL;
    // Passed in a default value
    if (argCount == 2) {
        defaultValue = args[2];
    }

    Value key = args[1];

    if (!IS_STRING(key)) {
        runtimeError(vm, "Argument passed to getAttribute() must be a string");
        return EMPTY_VAL;
    }

    ObjInstance *instance = AS_INSTANCE(args[0]);

    Value value;
    if (tableGet(&instance->publicFields, AS_STRING(key), &value)) {
        return value;
    }

    if (tableGet(&instance->klass->publicMethods, AS_STRING(key), &value)) {
        ObjBoundMethod *bound = newBoundMethod(vm, OBJ_VAL(instance), AS_CLOSURE(value));

        return OBJ_VAL(bound);
    }

    // Check class for properties
    ObjClass *klass = instance->klass;

    while (klass != NULL) {
        if (tableGet(&klass->publicProperties, AS_STRING(key), &value)) {
            return value;
        }

        klass = klass->superclass;
    }

    return defaultValue;
}

static Value setAttribute(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "setAttribute() takes 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Value value = args[2];
    Value key = args[1];

    if (!IS_STRING(key)) {
        runtimeError(vm, "Argument passed to setAttribute() must be a string");
        return EMPTY_VAL;
    }

    ObjInstance *instance = AS_INSTANCE(args[0]);
    tableSet(vm, &instance->publicFields, AS_STRING(key), value);

    return NIL_VAL;
}

static Value isInstance(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "isInstance() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_CLASS(args[1])) {
        runtimeError(vm, "Argument passed to isInstance() must be a class");
        return EMPTY_VAL;
    }

    ObjInstance *object = AS_INSTANCE(args[0]);

    ObjClass *klass = AS_CLASS(args[1]);
    ObjClass *klassToFind = object->klass;

    while (klassToFind != NULL) {
        if (klass == klassToFind) {
            return BOOL_VAL(true);
        }

        klassToFind = klassToFind->superclass;
    }

    return BOOL_VAL(false);
}

static Value copyShallow(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "copy() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjInstance *oldInstance = AS_INSTANCE(args[0]);
    ObjInstance *instance = copyInstance(vm, oldInstance, true);

    return OBJ_VAL(instance);
}

static Value copyDeep(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "deepCopy() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjInstance *oldInstance = AS_INSTANCE(args[0]);
    ObjInstance *instance = copyInstance(vm, oldInstance, false);

    return OBJ_VAL(instance);
}

static Value methods(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "methods() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjInstance *instance = AS_INSTANCE(args[0]);

    ObjList *list = newList(vm);
    push(vm, OBJ_VAL(list));

    for (int i = 0; i <= instance->klass->publicMethods.capacityMask; ++i) {
        if (instance->klass->publicMethods.entries[i].key == NULL) {
            continue;
        }

        writeValueArray(vm, &list->values, OBJ_VAL(instance->klass->publicMethods.entries[i].key));
    }
    pop(vm);

    return OBJ_VAL(list);
}

void declareInstanceMethods(DictuVM *vm) {
    defineNative(vm, &vm->instanceMethods, "toString", toString);
    defineNative(vm, &vm->instanceMethods, "hasAttribute", hasAttribute);
    defineNative(vm, &vm->instanceMethods, "getAttribute", getAttribute);
    defineNative(vm, &vm->instanceMethods, "setAttribute", setAttribute);
    defineNative(vm, &vm->instanceMethods, "isInstance", isInstance);
    defineNative(vm, &vm->instanceMethods, "copy", copyShallow);
    defineNative(vm, &vm->instanceMethods, "deepCopy", copyDeep);
    defineNative(vm, &vm->instanceMethods, "methods", methods);
}
