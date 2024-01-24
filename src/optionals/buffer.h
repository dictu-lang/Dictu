#ifndef dictu_buffer_h
#define dictu_buffer_h

#include <string.h>

#include "optionals.h"
#include "../vm/vm.h"
#include <stdint.h>

#define BUFFER_SIZE_MAX 2147483647

#define IS_BIG_ENDIAN (!*(unsigned char *)&(uint16_t){1})

Value createBufferModule(DictuVM *vm);

#endif //dictu_buffer_h