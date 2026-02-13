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

void grayQueue(DictuVM *vm, ObjAbstract *abstract) {
    Queue *queue = (Queue*)abstract->data;

    if (queue == NULL) return;

    if (queue->count == queue->capacity) {
        for (int i = queue->front; i < queue->capacity; ++i) {
            grayValue(vm, queue->dq[i]);
        }

        return;
    }

    if (queue->rear + 1 < queue->front) {
        for (int i = queue->front; i < queue->capacity; ++i) {
            grayValue(vm, queue->dq[i]);
        }

        for (int i = 0; i < queue->rear + 1; ++i) {
            grayValue(vm, queue->dq[i]);
        }
    } else {
        for (int i = queue->front; i < queue->rear + 1; ++i) {
            grayValue(vm, queue->dq[i]);
        }
    }
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
    bool isEmpty = queue->count == 0;

    return BOOL_VAL(isEmpty);
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

Queue* createQueue(DictuVM *vm, double capacity);

static void shrinkQueue(DictuVM *vm, Queue *queue) {
    int oldCapacity = queue->capacity;
    Queue *newQueue = createQueue(vm, SHRINK_CAPACITY(oldCapacity));

    while (queue->count != 0) {
        newQueue->dq[++newQueue->rear] = queue->dq[queue->front++];
        newQueue->count++;

        if (queue->front == queue->capacity) {
            queue->front = 0;
        }

        queue->count--;
    }

    FREE_ARRAY(vm, Value, queue->dq, queue->capacity);

    queue->dq = newQueue->dq;
    queue->capacity = newQueue->capacity;
    queue->front = newQueue->front;
    queue->rear = newQueue->rear;
    queue->count = newQueue->count;

    FREE(vm, Queue, newQueue);
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

    if (queue->count < queue->capacity / 2 && queue->capacity > DEFAULT_QUEUE_CAPACITY) {
        shrinkQueue(vm, queue);
    }

    Value data = queue->dq[queue->front++];

    if (queue->front == queue->capacity) {
        queue->front = 0;
    }

    queue->count--;

    return data;
}

Queue* createQueue(DictuVM *vm, double capacity) {
    Queue *queue = ALLOCATE(vm, Queue, 1);
    queue->dq = ALLOCATE(vm, Value, capacity);
    queue->capacity = capacity;
    queue->front = 0;
    queue->rear = -1;
    queue->count = 0;

    return queue;
}

ObjAbstract* newQueueObj(DictuVM *vm, double capacity) {
    ObjAbstract *abstract = newAbstract(vm, freeQueue, queueToString);
    push(vm, OBJ_VAL(abstract));

    Queue *queue = createQueue(vm, capacity);

    /**
     * Setup Queue object methods
     */
    defineNative(vm, &abstract->values, "isEmpty", queueIsEmpty);
    defineNative(vm, &abstract->values, "isFull", queueIsFull);
    defineNative(vm, &abstract->values, "cap", queueCap);
    defineNative(vm, &abstract->values, "len", queueLen);
    defineNative(vm, &abstract->values, "peek", queuePeek);
    defineNative(vm, &abstract->values, "push", queuePush);
    defineNative(vm, &abstract->values, "pop", queuePop);

    abstract->data = queue;
    abstract->grayFunc = grayQueue;
    pop(vm);

    return abstract;
}

static Value newQueue(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "new() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    return OBJ_VAL(newQueueObj(vm, DEFAULT_QUEUE_CAPACITY));
}

static Value newQueueWithSize(DictuVM *vm, int argCount, Value *args) {
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

    return newResultSuccess(vm, OBJ_VAL(newQueueObj(vm, capacity)));
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
