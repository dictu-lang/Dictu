#ifndef dictu_c_h
#define dictu_c_h

#ifndef _GNU_SOURCE
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#if (_POSIX_C_SOURCE >= 200112L)
#define POSIX_STRERROR
#endif
#endif

#include <string.h>
#include <errno.h>

#include "optionals.h"
#include "../vm.h"
#include "../memory.h"

void createCClass(VM *vm);

#define MAX_ERROR_LEN 256
Value strerrorGeneric(VM *, int);
Value strerrorNative(VM *, int, Value *);

#endif //dictu_c_h
