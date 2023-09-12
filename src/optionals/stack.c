#include <string.h>

#include "stack.h"
#include "optionals.h"
#include "../vm/vm.h"

typedef struct {
    Value *ds;
    int capacity;
    int top;
    int count;
} Stack;

#define AS_STACK(v) ((Stack*)AS_ABSTRACT(v)->data)
#define DEFAULT_STACK_CAPACITY 8

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
    bool isFull = stack->capacity == stack->count;

    return BOOL_VAL(isFull);
}

static Value stackIsEmpty(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "isEmpty() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Stack *stack = AS_STACK(args[0]);
    bool isEmpty = stack->count == 0;
    
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

    return NUMBER_VAL(stack->count);
}

static Value stackPeek(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "peek() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Stack *stack = AS_STACK(args[0]);
    
    return stack->ds[stack->top];
}

static Value stackPush(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "push() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    Stack *stack = AS_STACK(args[0]);
    if (stack->count == stack->capacity) {
        int oldCapacity = stack->capacity;
        stack->capacity = GROW_CAPACITY(oldCapacity);
        stack->ds = GROW_ARRAY(vm, stack->ds, Value, oldCapacity, stack->capacity);
    }

    stack->ds[stack->top] = args[1];
    stack->count++;

    return NIL_VAL;
}

static Value stackPop(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "pop() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Stack *stack = AS_STACK(args[0]);

    if (stack->count == 0) {
        runtimeError(vm, "pop() called on an empty stack");
        return EMPTY_VAL;
    }

    Value data = stack->ds[stack->top];
    stack->top--;
    stack->count--;

    if (stack->count < stack->capacity / 2 && stack->capacity > DEFAULT_STACK_CAPACITY) {
        int oldCapacity = stack->capacity;        
        stack->capacity = SHRINK_CAPACITY(oldCapacity);
        stack->ds = SHRINK_ARRAY(vm, stack->ds, Value, oldCapacity, stack->capacity);
    }

    return data;
}

ObjAbstract* newStackObj(DictuVM *vm) {
    ObjAbstract *abstract = newAbstract(vm, freeStack, stackToString);
    push(vm, OBJ_VAL(abstract));

    Stack *stack = ALLOCATE(vm, Stack, 1);

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
    pop(vm);

    return abstract;
}

static Value newStack(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "new() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjAbstract *abstract = newStackObj(vm);
    push(vm, OBJ_VAL(abstract));
    
    Stack *stack = abstract->data;

    stack->ds = ALLOCATE(vm, Value, DEFAULT_STACK_CAPACITY);
    stack->capacity = DEFAULT_STACK_CAPACITY;
    stack->top = 0;
    stack->count = 0;

    pop(vm);

    return OBJ_VAL(abstract);
}

static Value newStackWithSize(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "newWithSize() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError(vm, "newWithSize() argument must be a numbers");
        return EMPTY_VAL;
    }

    double capacity = AS_NUMBER(args[0]);
    if (capacity <= 0) {
        return newResultError(vm, "capacity must be greater than 0");
    }

    ObjAbstract *abstract = newStackObj(vm);
    push(vm, OBJ_VAL(abstract));
    
    Stack *stack = abstract->data;

    stack->ds = ALLOCATE(vm, Value, capacity);
    stack->capacity = capacity;
    stack->top = 0;
    stack->count = 0;

    Value success = newResultSuccess(vm, OBJ_VAL(abstract));
    pop(vm);

    return success;
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
