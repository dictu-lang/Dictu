#include <stdlib.h>
#include "chunk.h"
#include "memory.h"
#include "value.h"
#include "vm.h"

void initChunk(VM *vm, Chunk *chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    initValueArray(&chunk->constants);
}

void freeChunk(VM *vm, Chunk *chunk) {
    FREE_ARRAY(vm, uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(vm, int, chunk->lines, chunk->capacity);
    freeValueArray(vm, &chunk->constants);
    initChunk(vm, chunk);
}

void writeChunk(VM *vm, Chunk *chunk, uint8_t byte, int line) {
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(vm, chunk->code, uint8_t,
                                 oldCapacity, chunk->capacity);
        chunk->lines = GROW_ARRAY(vm, chunk->lines, int,
                                  oldCapacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

int addConstant(VM *vm, Chunk *chunk, Value value) {
    push(vm, value);

    writeValueArray(vm, &chunk->constants, value);
    pop(vm);
    return chunk->constants.count - 1;
}
