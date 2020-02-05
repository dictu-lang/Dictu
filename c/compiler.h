#ifndef dictu_compiler_h
#define dictu_compiler_h

#include "object.h"
#include "vm.h"

ObjFunction *compile(const char *source, bool repl);

void grayCompilerRoots();

#endif
