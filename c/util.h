#ifndef dictu_util_h
#define dictu_util_h

#include "vm.h"

char *readFile(const char *path);

void defineNative(VM *vm, Table *table, const char *name, NativeFn function);

bool isValidKey(Value value);

#endif //dictu_util_h
