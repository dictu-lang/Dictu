#ifndef dictu_path_h
#define dictu_path_h

#include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "optionals.h"
#include "../vm/vm.h"
#include "../vm/memory.h"

#ifdef _WIN32
#include "windowsapi.h"
#else
#include <dirent.h>
#endif

ObjModule *createPathModule(DictuVM *vm);

#endif //dictu_path_h
