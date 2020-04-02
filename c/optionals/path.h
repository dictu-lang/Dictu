#ifndef dictu_path_h
#define dictu_path_h

#ifdef _WIN32
#define DIR_SEPARATOR '\\'
#include <windows.h>
#else
#define DIR_SEPARATOR '/'
#endif

#define IS_DIR_SEPARATOR(c) (c == DIR_SEPARATOR)

#include "optionals.h"
#include "../vm.h"
#include "../memory.h"

void createPathClass();

#endif //dictu_path_h
