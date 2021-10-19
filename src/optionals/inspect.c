#include "inspect.h"

static Value getLine(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0 && argCount != 1) {
        runtimeError(vm, "getLine() takes takes 0 or 1 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    int count = 0;

    if (argCount == 1) {
        if (!IS_NUMBER(args[0])) {
            runtimeError(vm, "Optional argument passed to getLine() must be a number.");
            return EMPTY_VAL;
        }

        count = AS_NUMBER(args[0]);

        if (count < 0) {
            runtimeError(vm, "Optional argument passed to getLine() must be 0 or more.");
            return EMPTY_VAL;
        }

        if (count > vm->frameCount - 1) {
            runtimeError(vm, "Optional argument passed to getLine() exceeds the frame count.");
            return EMPTY_VAL;
        }
    }

    CallFrame *frame = &vm->frames[vm->frameCount - 1 - count];
    ObjFunction *function = frame->closure->function;
    size_t instruction = frame->ip - function->chunk.code - 1;
    
    return NUMBER_VAL(function->chunk.lines[instruction]);
}

static Value getFrameCount(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "getFrameCount() takes takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    // -1 as it's pointing to the next frame
    return NUMBER_VAL(vm->frameCount - 1);
}

Value createInspectModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Inspect", 7);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Inspect methods
     */
    defineNative(vm, &module->values, "getLine", getLine);
    defineNative(vm, &module->values, "getFrameCount", getFrameCount);

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}