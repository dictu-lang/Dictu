#ifndef dictu_system_h
#define dictu_system_h

#include <ctype.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#ifdef _WIN32
#include "windowsapi.h"
#include <direct.h>
#define REMOVE remove
#define MKDIR(d, m) ((void)m, _mkdir(d))
#else
#include <sys/utsname.h>
#include <unistd.h>
#define HAS_ACCESS
#define REMOVE unlink
#define MKDIR(d, m) mkdir(d, m)
#endif

#include "../vm/common.h"
#include "../vm/memory.h"
#include "../vm/vm.h"
#include "optionals.h"
#include "arch.h"

Value createSystemModule(DictuVM *vm);

#endif // dictu_system_h
