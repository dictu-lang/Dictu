#ifndef dictu_util_h
#define dictu_util_h

#include "vm.h"

char *readFile(DictuVM *vm, const char *path);

ObjString *getDirectory(DictuVM *vm, char *source);

void defineNative(DictuVM *vm, Table *table, const char *name, NativeFn function);

void defineNativeProperty(DictuVM *vm, Table *table, const char *name, Value value);

bool isValidKey(Value value);

Value boolNative(DictuVM *vm, int argCount, Value *args);

ObjString *dirname(DictuVM *vm, char *path, int len);

bool resolvePath(char *directory, char *path, char *ret);

#endif //dictu_util_h
