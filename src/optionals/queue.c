#include <string.h>

#include "queue.h"
#include "optionals.h"
#include "../vm/vm.h"

typedef struct {
    Value *dq;
    int capacity;
    int front;
    int rear;
    int count;
} Queue;

#define AS_QUEUE(v) ((Queue*)AS_ABSTRACT(v)->data)
#define DEFAULT_QUEUE_CAPACITY 8

void freeQueue(DictuVM *vm, ObjAbstract *abstract) {
    Queue *queue = (Queue*)abstract->data;
    FREE_ARRAY(vm, Value, queue->dq, queue->capacity);
    FREE(vm, Queue, abstract->data);
}

char *queueToString(ObjAbstract *abstract) {
    UNUSED(abstract);

    char *queueString = malloc(sizeof(char) * 8);
    snprintf(queueString, 8, "<Queue>");
    return queueString;
}


static Value queueIsFull(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "isFull() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Queue *queue = AS_QUEUE(args[0]);
    bool isFull = queue->capacity == queue->count;

    return BOOL_VAL(isFull);
}

static Value queueIsEmpty(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "isEmpty() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Queue *queue = AS_QUEUE(args[0]);
    bool isFull = queue->count == 0;

    return BOOL_VAL(isFull);
}

static Value queueCap(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "cap() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Queue *queue = AS_QUEUE(args[0]);

    return NUMBER_VAL(queue->capacity);
}

static Value queueLen(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "len() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Queue *queue = AS_QUEUE(args[0]);

    return NUMBER_VAL(queue->count);
}

static Value queuePeek(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "peek() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Queue *queue = AS_QUEUE(args[0]);
    
    return queue->dq[queue->front];
}

static Value queuePush(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "push() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    Queue *queue = AS_QUEUE(args[0]);

    if (queue->capacity < queue->count + 1) {
        int oldCapacity = queue->capacity;
        queue->capacity = GROW_CAPACITY(oldCapacity);
        queue->dq = GROW_ARRAY(vm, queue->dq, Value, oldCapacity, queue->capacity);
    }

    if (queue->rear == queue->capacity-1) {
        queue->rear = -1;            
    }

    queue->dq[++queue->rear] = args[1];
    queue->count++;

    return NIL_VAL;
}

static Value queuePop(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "pop() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Queue *queue = AS_QUEUE(args[0]);

    if (queue->count == 0) {
        runtimeError(vm, "pop() called on an empty queue");
        return EMPTY_VAL;
    }

    Value data = queue->dq[queue->front++];

    if (queue->front == queue->capacity) {
        queue->front = 0;
    }

    queue->count--;

    if (queue->count < queue->capacity / 2 && queue->capacity > DEFAULT_QUEUE_CAPACITY) {
        int oldCapacity = queue->capacity;
        queue->capacity = SHRINK_CAPACITY(oldCapacity);
        queue->dq = SHRINK_ARRAY(vm, queue->dq, Value, oldCapacity, queue->capacity);
    }

    return data;
}

ObjAbstract* newQueueObj(DictuVM *vm) {
    ObjAbstract *abstract = newAbstract(vm, freeQueue, queueToString);
    push(vm, OBJ_VAL(abstract));

    Queue *queue = ALLOCATE(vm, Queue, 1);

    /**
     * Setup Queue object methods
     */
    defineNative(vm, &abstract->values, "isEmpty", queueIsEmpty);
    defineNative(vm, &abstract->values, "isFull", queueIsFull);
    defineNative(vm, &abstract->values, "cap", queueCap);
    defineNative(vm, &abstract->values, "len", queueLen);
    defineNative(vm, &abstract->values, "peek",  queuePeek);
    defineNative(vm, &abstract->values, "push", queuePush);
    defineNative(vm, &abstract->values, "pop", queuePop);

    abstract->data = queue;
    pop(vm);

    return abstract;
}

static Value newQueue(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "new() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjAbstract *abstract = newQueueObj(vm);
    push(vm, OBJ_VAL(abstract));
    
    Queue *queue = abstract->data;

    queue->dq = ALLOCATE(vm, Value, DEFAULT_QUEUE_CAPACITY);
    queue->capacity = DEFAULT_QUEUE_CAPACITY;
    queue->front = 0;
    queue->rear = -1;
    queue->count = 0;

    pop(vm);

    return OBJ_VAL(abstract);
}

static Value newQueueWithSize(DictuVM *vm, int argCount, Value *args) {
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

    ObjAbstract *abstract = newQueueObj(vm);
    push(vm, OBJ_VAL(abstract));
    
    Queue *queue = abstract->data;

    queue->dq = ALLOCATE(vm, Value, capacity);
    queue->capacity = capacity;
    queue->front = 0;
    queue->rear = -1;
    queue->count = 0;

    Value success = newResultSuccess(vm, OBJ_VAL(abstract));
    pop(vm);

    return success;
}

Value createQueueModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Queue", 5);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Queue methods
     */
    defineNative(vm, &module->values, "new", newQueue);
    defineNative(vm, &module->values, "newWithSize", newQueueWithSize);

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
