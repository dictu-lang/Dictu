#include "bool.h"
#include "../vm.h"

static Value toStringBool(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "toString() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    int val = AS_BOOL(args[0]);
    return OBJ_VAL(copyString(vm, val ? "true" : "false", val ? 4 : 5));
}

void declareBoolMethods(DictuVM *vm) {
    defineNative(vm, &vm->boolMethods, "toString", toStringBool);
}
