#include "class.h"
#include "../vm.h"
#include "../memory.h"

static Value toString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "toString() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    char *valueString = classToString(args[0]);

    ObjString *string = copyString(vm, valueString, strlen(valueString));
    free(valueString);

    return OBJ_VAL(string);
}

static Value methods(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "methods() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjClass *klass = AS_CLASS(args[0]);

    ObjList *list = newList(vm);
    push(vm, OBJ_VAL(list));

    for (int i = 0; i <= klass->publicMethods.capacityMask; ++i) {
        if (klass->publicMethods.entries[i].key == NULL) {
            continue;
        }

        writeValueArray(vm, &list->values, OBJ_VAL(klass->publicMethods.entries[i].key));
    }
    pop(vm);

    return OBJ_VAL(list);
}

void declareClassMethods(DictuVM *vm) {
    defineNative(vm, &vm->classMethods, "toString", toString);
    defineNative(vm, &vm->classMethods, "methods", methods);
}
