#ifndef dictu_system_h
#define dictu_system_h

#include <time.h>
#include <limits.h>
#include <sys/utsname.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "optionals.h"
#include "../vm.h"
#include "../memory.h"

void createSystemClass(VM *vm, int argc, const char *argv[]);

#endif //dictu_system_h
