#ifndef dictu_math_h
#define dictu_math_h

#include <string.h>
#include <math.h>

#include "optionals.h"
#include "../vm/vm.h"

#define FLOAT_TOLERANCE 0.00001

ObjModule *createMathsModule(DictuVM *vm);

#endif //dictu_math_h
