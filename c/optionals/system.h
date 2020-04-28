#ifndef dictu_system_h
#define dictu_system_h

#include <time.h>
#include <limits.h>
#include <sys/utsname.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#define REMOVE remove
#else
#include <unistd.h>
#define REMOVE unlink
#endif

#include "optionals.h"
#include "../vm.h"
#include "../memory.h"

void createSystemClass(VM *vm, int argc, const char *argv[]);

#endif //dictu_system_h
