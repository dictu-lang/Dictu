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
    if (tableGet(&instance->publicAttributes, AS_STRING(value), &_)) {
        return TRUE_VAL;
    }

    if (tableGet(&instance->klass->publicMethods, AS_STRING(value), &value)) {
        return TRUE_VAL;
    }

    // Check class for attributes
    ObjClass *klass = instance->klass;

    while (klass != NULL) {
        if (tableGet(&klass->constants, AS_STRING(value), &value)) {
            return TRUE_VAL;
        }

        if (tableGet(&klass->variables, AS_STRING(value), &value)) {
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
    if (tableGet(&instance->publicAttributes, AS_STRING(key), &value)) {
        return value;
    }

    if (tableGet(&instance->klass->publicMethods, AS_STRING(key), &value)) {
        ObjBoundMethod *bound = newBoundMethod(vm, OBJ_VAL(instance), AS_CLOSURE(value));

        return OBJ_VAL(bound);
    }

    // Check class for properties
    ObjClass *klass = instance->klass;

    while (klass != NULL) {
        if (tableGet(&klass->variables, AS_STRING(key), &value)) {
            return value;
        }

        klass = klass->superclass;
    }

    return defaultValue;
}

static bool exists(ObjList *list, ObjString *search) {
    for (int i = 0; i < list->values.count; ++i) {
        if (valuesEqual(list->values.values[i], OBJ_VAL(search))) {
            return true;
        }
    }

    return false;
}

static Value getAttributes(DictuVM *vm, int argCount, Value *args) {
    if (argCount > 0) {
        runtimeError(vm, "getAttributes() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjInstance *instance = AS_INSTANCE(args[0]);

    ObjDict *dict = newDict(vm);
    push(vm, OBJ_VAL(dict));

    ObjList *fields = newList(vm);
    push(vm, OBJ_VAL(fields));

    ObjClass *klass = instance->klass;

    // Walk the inheritance chain
    while (klass != NULL) {
        for (int i = 0; i < klass->variables.capacity; i++) {
            if (klass->variables.entries[i].key == NULL) {
                continue;
            }

            if (exists(fields, klass->variables.entries[i].key)) {
                continue;
            }

            writeValueArray(vm, &fields->values, OBJ_VAL(klass->variables.entries[i].key));
        }

        for (int i = 0; i < klass->constants.capacity; i++) {
            if (klass->constants.entries[i].key == NULL) {
                continue;
            }

            if (exists(fields, klass->constants.entries[i].key)) {
                continue;
            }

            writeValueArray(vm, &fields->values, OBJ_VAL(klass->constants.entries[i].key));
        }

        klass = klass->superclass;
    }

    ObjString *cv = copyString(vm, "fields", 6);
    push(vm, OBJ_VAL(cv));

    dictSet(vm, dict, OBJ_VAL(cv), OBJ_VAL(fields));

    pop(vm); // "fields" string
    pop(vm); // "fields" list

    ObjList *attributes = newList(vm);
    push(vm, OBJ_VAL(attributes));
    
    for (int i = 0; i < instance->publicAttributes.capacity; i++) {
        if (instance->publicAttributes.entries[i].key == NULL) {
            continue;
        }

        if (exists(attributes, instance->publicAttributes.entries[i].key)) {
            continue;
        }

        writeValueArray(vm, &attributes->values, OBJ_VAL(instance->publicAttributes.entries[i].key));
    }

    ObjString *pv = copyString(vm, "attributes", 10);
    push(vm, OBJ_VAL(pv));

    dictSet(vm, dict, OBJ_VAL(pv), OBJ_VAL(attributes));

    pop(vm); // "attributes" string
    pop(vm); // "attributes" list

    ObjList *methods = newList(vm);
    push(vm, OBJ_VAL(methods));

    for (int i = 0; i < instance->klass->publicMethods.capacity; ++i) {
        if (instance->klass->publicMethods.entries[i].key == NULL) {
            continue;
        }

        writeValueArray(vm, &methods->values, OBJ_VAL(instance->klass->publicMethods.entries[i].key));
    }
    ObjString *mv = copyString(vm, "methods", 7);
    push(vm, OBJ_VAL(mv));

    dictSet(vm, dict, OBJ_VAL(mv), OBJ_VAL(methods));

    pop(vm); // "methods" string
    pop(vm); // "methods" list

    pop(vm); // dict

    return OBJ_VAL(dict);
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
    tableSet(vm, &instance->publicAttributes, AS_STRING(key), value);

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

    for (int i = 0; i < instance->klass->publicMethods.capacity; ++i) {
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
    defineNative(vm, &vm->instanceMethods, "getAttributes", getAttributes);
    defineNative(vm, &vm->instanceMethods, "setAttribute", setAttribute);
    defineNative(vm, &vm->instanceMethods, "isInstance", isInstance);
    defineNative(vm, &vm->instanceMethods, "copy", copyShallow);
    defineNative(vm, &vm->instanceMethods, "deepCopy", copyDeep);
    defineNative(vm, &vm->instanceMethods, "methods", methods);
}
