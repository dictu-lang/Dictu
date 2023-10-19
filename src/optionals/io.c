#include "io.h"


static Value printIO(DictuVM *vm, int argCount, Value *args) {
    if (argCount == 0) {
        runtimeError(vm, "print() takes 1 or more arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    for (int i = 0; i < argCount; ++i) {
        printValue(args[i]);
    }

    return NIL_VAL;
}

static Value printlnIO(DictuVM *vm, int argCount, Value *args) {
    if (argCount == 0) {
        runtimeError(vm, "println() takes 1 or more arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    for (int i = 0; i < argCount; ++i) {
        printValue(args[i]);
        printf("\n");
    }

    return NIL_VAL;
}

Value createIOModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "IO", 2);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    defineNativeProperty(vm, &module->values, "stdin",  NUMBER_VAL(STDIN_FILENO));
    defineNativeProperty(vm, &module->values, "stdout",  NUMBER_VAL(STDOUT_FILENO));
    defineNativeProperty(vm, &module->values, "stderr",  NUMBER_VAL(STDERR_FILENO));

    /**
     * Define IO methods
     */
    defineNative(vm, &module->values, "print", printIO);
    defineNative(vm, &module->values, "println", printlnIO);

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
