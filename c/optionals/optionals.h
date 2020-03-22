#ifndef dictu_optionals_h
#define dictu_optionals_h

#include "math.h"
#include "env.h"
#include "system.h"
#include "json.h"
#include "http.h"

#include "../vm.h"

void defineNativeMethod(VM *vm, ObjClassNative *klass, const char *name, NativeFn function);

#endif //dictu_optionals_h
