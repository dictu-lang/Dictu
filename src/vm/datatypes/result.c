#include "result.h"

static Value unwrap(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "unwrap() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjResult *result = AS_RESULT(args[0]);

    if (result->status == ERR) {
        runtimeError(vm, "Attempted unwrap() on an error Result value '%s'", AS_CSTRING(result->value));
        return EMPTY_VAL;
    }

    return result->value;
}

static Value unwrapError(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "unwrapError() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjResult *result = AS_RESULT(args[0]);

    if (result->status == SUCCESS) {
        runtimeError(vm, "Attempted unwrapError() on a success Result value");
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
    return BOOL_VAL(result->status == SUCCESS);
}

void declareResultMethods(DictuVM *vm) {
    defineNative(vm, &vm->resultMethods, "unwrap", unwrap);
    defineNative(vm, &vm->resultMethods, "unwrapError", unwrapError);
    defineNative(vm, &vm->resultMethods, "success", success);
}
