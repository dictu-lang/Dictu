#include "class.h"
#include "../vm.h"
#include "../memory.h"

static Value toString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "toString() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    int valueStringLen = 0;
    char *valueString = classToString(vm, args[0], &valueStringLen);

    ObjString *string = copyString(vm, valueString, valueStringLen);
    FREE_ARRAY(vm, char, valueString, valueStringLen + 1);

    return OBJ_VAL(string);
}

static Value toDict(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "toDict() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjDict *dict = classToDict(vm, args[0]);

    return OBJ_VAL(dict);
}

static Value methods(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "methods() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjClass *klass = AS_CLASS(args[0]);

    ObjList *list = newList(vm);
    push(vm, OBJ_VAL(list));

    for (int i = 0; i < klass->publicMethods.capacity; ++i) {
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
    defineNative(vm, &vm->classMethods, "toDict", toDict);
    defineNative(vm, &vm->classMethods, "methods", methods);
}
