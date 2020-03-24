#include "optionals.h"

void defineNativeMethod(VM *vm, ObjClassNative *klass, const char *name, NativeFn function) {
    ObjNative *native = newNative(vm, function);
    push(vm, OBJ_VAL(native));
    ObjString *methodName = copyString(vm, name, strlen(name));
    push(vm, OBJ_VAL(methodName));
    tableSet(vm, &klass->methods, methodName, OBJ_VAL(native));
    pop(vm);
    pop(vm);
}

