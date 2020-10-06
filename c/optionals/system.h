#ifndef dictu_system_h
#define dictu_system_h

#include <time.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>

#ifdef _WIN32
#include "../windowsapi.h"
#include <direct.h>
#define REMOVE remove
#define MKDIR(d, m) ((void)m, _mkdir(d))
#else
#include <unistd.h>
#include <sys/utsname.h>
#define HAS_ACCESS
#define REMOVE unlink
#define MKDIR(d, m) mkdir(d, m)
#endif

#include "optionals.h"
#include "../vm.h"
#include "../memory.h"

void createSystemClass(VM *vm, int argc, const char *argv[]);

#endif //dictu_system_h
