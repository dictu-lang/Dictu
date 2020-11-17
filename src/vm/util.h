#ifndef dictu_util_h
#define dictu_util_h

#include "vm.h"

char *readFile(DictuVM *vm, const char *path);

void defineNative(DictuVM *vm, Table *table, const char *name, NativeFn function);

void defineNativeProperty(DictuVM *vm, Table *table, const char *name, Value value);

bool isValidKey(Value value);

Value boolNative(DictuVM *vm, int argCount, Value *args);

#endif //dictu_util_h
