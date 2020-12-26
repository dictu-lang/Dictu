#ifndef dictu_memory_h
#define dictu_memory_h

#include "object.h"
#include "common.h"

#define ALLOCATE(vm, type, count) \
    (type*)reallocate(vm, NULL, 0, sizeof(type) * (count))

#define FREE(vm, type, pointer) \
    reallocate(vm, pointer, sizeof(type), 0)

#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define SHRINK_CAPACITY(capacity) \
    ((capacity) < 16 ? 8 : (capacity) / 2)

#define GROW_ARRAY(vm, previous, type, oldCount, count) \
    (type*)reallocate(vm, previous, sizeof(type) * (oldCount), \
        sizeof(type) * (count))

#define SHRINK_ARRAY(vm, previous, type, oldCount, count) \
    (type*)reallocate(vm, previous, sizeof(type) * (oldCount), \
        sizeof(type) * (count))

#define FREE_ARRAY(vm, type, pointer, oldCount) \
    reallocate(vm, pointer, sizeof(type) * (oldCount), 0)

void *reallocate(DictuVM *vm, void *previous, size_t oldSize, size_t newSize);

void grayObject(DictuVM *vm, Obj *object);

void grayValue(DictuVM *vm, Value value);

void collectGarbage(DictuVM *vm);

void freeObjects(DictuVM *vm);

void freeObject(DictuVM *vm, Obj *object);

#endif
