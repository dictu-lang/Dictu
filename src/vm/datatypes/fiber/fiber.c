#include "fiber.h"

/**
 * fiber.call(value?) -> value
 *
 * Resumes the fiber, optionally passing a value. On first call, the value
 * becomes the parameter to the fiber's function. On subsequent calls
 * (after a yield), it becomes the return value of yield() inside the fiber.
 *
 * Returns the value yielded or returned by the fiber.
 */
static Value fiberCall(DictuVM *vm, int argCount, Value *args) {
    if (argCount > 1) {
        runtimeError(vm, "call() takes 0 or 1 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    ObjFiber *fiber = AS_FIBER(args[0]);

    if (fiber->state == FIBER_DONE) {
        runtimeError(vm, "Cannot call a finished fiber.");
        return EMPTY_VAL;
    }

    if (fiber->state == FIBER_RUNNING) {
        runtimeError(vm, "Fiber is already running.");
        return EMPTY_VAL;
    }

    Value passedValue = (argCount == 1) ? args[1] : NIL_VAL;

    ObjFiber *caller = vm->fiber;

    // Set up the caller chain so yield/return knows where to go back.
    fiber->caller = caller;

    // Prepare the caller's stack: collapse the call() invocation down to
    // a single result slot. callNativeMethod normally does this, but since
    // we're switching fibers, we do it ourselves.
    // Stack before: [..., fiberObj, arg?]
    // Stack after:  [..., <placeholder>]  (placeholder will be overwritten by yield/return)
    caller->stackTop -= argCount + 1;
    push(vm, NIL_VAL); // placeholder for result

    // Deliver the value to the target fiber.
    if (fiber->frameCount == 1 && fiber->frames[0].ip == fiber->frames[0].closure->function->chunk.code) {
        // First call: the fiber hasn't started yet.
        // If the function takes a parameter, place it in slot 1.
        if (fiber->frames[0].closure->function->arity == 1) {
            // slot 0 is the closure itself, slot 1 is the parameter
            if (fiber->stackTop == fiber->stack + 1) {
                // Need to add the parameter slot
                *fiber->stackTop++ = passedValue;
            } else {
                fiber->stack[1] = passedValue;
            }
        }
    } else {
        // Resume from yield: overwrite the yield() return value.
        // When yield() was called, it returned EMPTY_VAL and set fiberSwitch.
        // The target fiber's callNativeMethod already collapsed the stack and
        // pushed a placeholder. We overwrite that placeholder.
        fiber->stackTop[-1] = passedValue;
    }

    // Switch fibers.
    caller->state = FIBER_READY;
    fiber->state = FIBER_RUNNING;
    vm->fiber = fiber;
    vm->fiberSwitch = true;

    return EMPTY_VAL;
}

/**
 * fiber.isDone() -> bool
 *
 * Returns true if the fiber has completed execution.
 */
static Value fiberIsDone(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "isDone() takes 0 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    ObjFiber *fiber = AS_FIBER(args[0]);
    return BOOL_VAL(fiber->state == FIBER_DONE);
}

/**
 * fiber.toString() -> string
 *
 * Returns a string representation of the fiber.
 */
static Value fiberToString(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "toString() takes no arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    return OBJ_VAL(copyString(vm, "<Fiber>", 7));
}

void declareFiberMethods(DictuVM *vm) {
    defineNative(vm, &vm->fiberMethods, "call", fiberCall);
    defineNative(vm, &vm->fiberMethods, "isDone", fiberIsDone);
    defineNative(vm, &vm->fiberMethods, "toString", fiberToString);
}
