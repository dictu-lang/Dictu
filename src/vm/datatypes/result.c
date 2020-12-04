#include "result.h"

static Value unwrap(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "unwrap() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjResult *result = AS_RESULT(args[0]);

    if (result->type == ERROR) {
        runtimeError(vm, "Attempted unwrap() on an error value '%s'", AS_CSTRING(result->value));
        return EMPTY_VAL;
    }

    return result->value;
}

static Value success(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "success() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjResult *result = AS_RESULT(args[0]);
    return BOOL_VAL(result->type == SUCCESS);
}

void declareResultMethods(DictuVM *vm) {
    defineNative(vm, &vm->resultMethods, "unwrap", unwrap);
    defineNative(vm, &vm->resultMethods, "success", success);
}
