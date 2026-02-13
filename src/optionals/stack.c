#include <string.h>

#include "stack.h"
#include "optionals.h"
#include "../vm/vm.h"

typedef struct {
    Value *ds;
    int capacity;
    int top;
} Stack;

#define AS_STACK(v) ((Stack*)AS_ABSTRACT(v)->data)
#define DEFAULT_STACK_CAPACITY 8

void grayStack(DictuVM *vm, ObjAbstract *abstract) {
    Stack *stack = (Stack*)abstract->data;

    if (stack == NULL) return;

    for (int i = 0; i < stack->top; i++) {
        grayValue(vm, stack->ds[i]);
    }
}

void freeStack(DictuVM *vm, ObjAbstract *abstract) {
    Stack *stack = (Stack*)abstract->data;
    FREE_ARRAY(vm, Value, stack->ds, stack->capacity);
    FREE(vm, Stack, abstract->data);
}

char *stackToString(ObjAbstract *abstract) {
    UNUSED(abstract);

    char *stackString = malloc(sizeof(char) * 8);
    snprintf(stackString, 8, "<Stack>");
    return stackString;
}


static Value stackIsFull(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "isFull() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Stack *stack = AS_STACK(args[0]);
    bool isFull = stack->capacity == stack->top;

    return BOOL_VAL(isFull);
}

static Value stackIsEmpty(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "isEmpty() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Stack *stack = AS_STACK(args[0]);
    bool isEmpty = stack->top == 0;
    
    return BOOL_VAL(isEmpty);
}

static Value stackCap(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "cap() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Stack *stack = AS_STACK(args[0]);
    return NUMBER_VAL(stack->capacity);
}

static Value stackLen(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "len() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Stack *stack = AS_STACK(args[0]);

    return NUMBER_VAL(stack->top);
}

static Value stackPeek(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "peek() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Stack *stack = AS_STACK(args[0]);
    
    return stack->ds[stack->top - 1];
}

static Value stackPush(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "push() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    Stack *stack = AS_STACK(args[0]);
    if (stack->top == stack->capacity) {
        int oldCapacity = stack->capacity;
        stack->capacity = GROW_CAPACITY(oldCapacity);
        stack->ds = GROW_ARRAY(vm, stack->ds, Value, oldCapacity, stack->capacity);
    }

    stack->ds[stack->top++] = args[1];

    return NIL_VAL;
}

static Value stackPop(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "pop() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Stack *stack = AS_STACK(args[0]);

    if (stack->top == 0) {
        runtimeError(vm, "pop() called on an empty stack");
        return EMPTY_VAL;
    }

    Value data = stack->ds[--stack->top];

    if (stack->top < stack->capacity / 2 && stack->capacity > DEFAULT_STACK_CAPACITY) {
        int oldCapacity = stack->capacity;
        stack->capacity = SHRINK_CAPACITY(oldCapacity);
        stack->ds = SHRINK_ARRAY(vm, stack->ds, Value, oldCapacity, stack->capacity);
    }

    return data;
}

ObjAbstract* newStackObj(DictuVM *vm, double capacity) {
    ObjAbstract *abstract = newAbstract(vm, freeStack, stackToString);
    push(vm, OBJ_VAL(abstract));

    Stack *stack = ALLOCATE(vm, Stack, 1);
    stack->top = 0;
    stack->ds = ALLOCATE(vm, Value, capacity);
    stack->capacity = capacity;

    /**
     * Setup Stack object methods
     */
    defineNative(vm, &abstract->values, "isEmpty", stackIsEmpty);
    defineNative(vm, &abstract->values, "isFull", stackIsFull);
    defineNative(vm, &abstract->values, "cap", stackCap);
    defineNative(vm, &abstract->values, "len", stackLen);
    defineNative(vm, &abstract->values, "peek",  stackPeek);
    defineNative(vm, &abstract->values, "push", stackPush);
    defineNative(vm, &abstract->values, "pop", stackPop);

    abstract->data = stack;
    abstract->grayFunc = grayStack;

    pop(vm);

    return abstract;
}

static Value newStack(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "new() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    return OBJ_VAL(newStackObj(vm, DEFAULT_STACK_CAPACITY));
}

static Value newStackWithSize(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "newWithSize() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError(vm, "newWithSize() argument must be a number");
        return EMPTY_VAL;
    }

    double capacity = AS_NUMBER(args[0]);
    if (capacity <= 0) {
        return newResultError(vm, "capacity must be greater than 0");
    }

    return OBJ_VAL(newResultSuccess(vm, OBJ_VAL(newStackObj(vm, capacity))));
}

Value createStackModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Stack", 5);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Stack methods
     */
    defineNative(vm, &module->values, "new", newStack);
    defineNative(vm, &module->values, "newWithSize", newStackWithSize);

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
