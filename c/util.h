#ifndef dictu_util_h
#define dictu_util_h

#include "vm.h"

char *readFile(VM *vm, const char *path);

void defineNative(VM *vm, Table *table, const char *name, NativeFn function);

void defineNativeProperty(VM *vm, Table *table, const char *name, Value value);

bool isValidKey(Value value);

Value boolNative(VM *vm, int argCount, Value *args);

#endif //dictu_util_h
