#include "fiber.h"

/**
 * Fiber.new(fn) -> Fiber
 *
 * Creates a new fiber from a closure. The closure must take 0 or 1 parameters.
 * If it takes 1 parameter, the value passed to the first call() will be
 * delivered as that parameter.
 */
static Value fiberNew(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "new() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_CLOSURE(args[0])) {
        runtimeError(vm, "new() argument must be a function.");
        return EMPTY_VAL;
    }

    ObjClosure *closure = AS_CLOSURE(args[0]);
    if (closure->function->arity > 1) {
        runtimeError(vm, "Fiber function cannot take more than 1 parameter (%d given).",
                     closure->function->arity);
        return EMPTY_VAL;
    }

    ObjFiber *fiber = newFiber(vm, closure);
    return OBJ_VAL(fiber);
}

/**
 * Fiber.yield(value?) -> value
 *
 * Suspends the current fiber and returns control to its caller.
 * The optional value is returned to the caller as the result of call().
 * When the fiber is resumed, yield() returns the value passed to the next call().
 */
static Value fiberYield(DictuVM *vm, int argCount, Value *args) {
    if (argCount > 1) {
        runtimeError(vm, "yield() takes 0 or 1 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    ObjFiber *current = vm->fiber;

    if (current->caller == NULL) {
        runtimeError(vm, "Cannot yield from the main fiber.");
        return EMPTY_VAL;
    }

    // The value to yield back to the caller.
    Value yieldValue = (argCount == 1) ? args[0] : NIL_VAL;

    ObjFiber *caller = current->caller;

    // Place the yield value where the caller expects the result of call().
    // The caller's stack was collapsed to [..., placeholder] by fiberCall.
    caller->stackTop[-1] = yieldValue;

    // Collapse the current fiber's stack: replace the yield() call
    // (native fn + args) with a single placeholder slot.
    // When this fiber is resumed, fiberCall will overwrite stackTop[-1]
    // with the value passed to call(), which becomes yield()'s return value.
    // callValue normally does: stackTop -= argCount + 1; push(result);
    // We do the same but with a placeholder since fiberSwitch skips that.
    // Must be done BEFORE switching vm->fiber since push() uses vm->fiber.
    current->stackTop -= argCount + 1;
    *current->stackTop++ = NIL_VAL; // placeholder — overwritten by next call()

    // Suspend current fiber, resume caller.
    current->state = FIBER_READY;
    current->caller = NULL;
    caller->state = FIBER_RUNNING;
    vm->fiber = caller;
    vm->fiberSwitch = true;

    return EMPTY_VAL;
}

/**
 * Fiber.isMain() -> bool
 *
 * Returns true if the currently executing fiber is the main fiber
 * (i.e., it has no caller and was the first fiber created).
 */
static Value fiberIsMain(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "isMain() takes 0 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    // The main fiber is the one with no caller and state FIBER_RUNNING.
    // More precisely, it was created by newMainFiber and never has a caller.
    return BOOL_VAL(vm->fiber->caller == NULL && vm->fiber->state == FIBER_RUNNING);
}

Value createFiberModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Fiber", 5);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Fiber module methods
     */
    defineNative(vm, &module->values, "new", fiberNew);
    defineNative(vm, &module->values, "yield", fiberYield);
    defineNative(vm, &module->values, "isMain", fiberIsMain);

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
