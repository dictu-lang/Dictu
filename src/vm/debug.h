#ifndef dictu_debug_h
#define dictu_debug_h

#include "chunk.h"

void disassembleChunk(DictuVM *vm, Chunk *chunk, const char *name);

int disassembleInstruction(DictuVM *vm, Chunk *chunk, int offset);

#endif
