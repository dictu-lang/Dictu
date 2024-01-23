#ifndef dictu_buffer_h
#define dictu_buffer_h

#include <string.h>
#include <math.h>

#include "optionals.h"
#include "../vm/vm.h"

#define BUFFER_SIZE_MAX 2147483647

Value createBufferModule(DictuVM *vm);

#endif //dictu_math_h