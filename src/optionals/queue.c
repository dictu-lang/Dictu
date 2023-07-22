#include <string.h>

#include "queue.h"
#include "optionals.h"
#include "../vm/vm.h"

typedef struct {
    void **dq;
    int size;
    int front;
    int rear;
    int count;
} Queue;

#define AS_QUEUE(v) ((Queue*)AS_ABSTRACT(v)->data)

void freeQueue(DictuVM *vm, ObjAbstract *abstract) {
    Queue *queue = (Queue*)abstract->data;
    FREE(vm, void**, queue->dq);
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
        runtimeError(vm, "size() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Queue *queue = AS_QUEUE(args[0]);
    bool isFull = queue->size == queue->count;

    return BOOL_VAL(isFull);
}

static Value queueIsEmpty(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "empty() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Queue *queue = AS_QUEUE(args[0]);
    bool isFull = queue->count == 0;

    return BOOL_VAL(isFull);
}

static Value queueSize(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "size() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Queue *queue = AS_QUEUE(args[0]);

    return NUMBER_VAL(queue->size);
}

static Value queuePeek(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "peek() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Queue *queue = AS_QUEUE(args[0]);
    double *d = queue->dq[queue->front];
    
    return newResultSuccess(vm, NUMBER_VAL(*d));
}

static Value queuePush(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "push() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    Queue *queue = AS_QUEUE(args[0]);

    if (queue->size != queue->count) {
        if(queue->rear == queue->size-1) {
            queue->rear = -1;            
        }

        double *data = ALLOCATE(vm, double, 1);
        *data = AS_NUMBER(args[1]);

        queue->dq[++queue->rear] = data;
        queue->count++;

        return newResultSuccess(vm, NIL_VAL);
    }

    return newResultSuccess(vm, NIL_VAL);
}

static Value queuePop(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "pop() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Queue *queue = AS_QUEUE(args[0]);

    void *data = queue->dq[queue->front++];

    if(queue->front == queue->size) {
        queue->front = 0;
    }

    queue->count--;
    double *d = data;

    return newResultSuccess(vm, NUMBER_VAL(*d));
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
    defineNative(vm, &abstract->values, "size", queueSize);
    defineNative(vm, &abstract->values, "peek",  queuePeek);
    defineNative(vm, &abstract->values, "push", queuePush);
    defineNative(vm, &abstract->values, "pop", queuePop);

    abstract->data = queue;
    pop(vm);

    return abstract;
}

static Value newQueue(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "new() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError(vm, "new() argument must be a numbers");
        return EMPTY_VAL;
    }

    double size = AS_NUMBER(args[0]);
    if (size == 0) {
        return newResultError(vm, "error: size must be greater than 0");
    }

    ObjAbstract *abstract = newQueueObj(vm);
    Queue *queue = abstract->data;
    queue->dq = ALLOCATE(vm, void*, size);
    queue->size = size;
    queue->front = 0;
    queue->rear = -1;
    abstract->data = queue;

    push(vm, OBJ_VAL(abstract));
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

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
