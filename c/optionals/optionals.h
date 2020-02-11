#ifndef dictu_optionals_h
#define dictu_optionals_h

#include "math.h"
#include "env.h"
#include "../vm.h"

void defineNativeMethod(ObjClassNative *klass, const char *name, NativeFn function);

void defineNativeVoidMethod(ObjClassNative *klass, const char *name, NativeFnVoid function);

#endif //dictu_optionals_h
