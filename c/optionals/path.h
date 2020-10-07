#ifndef dictu_path_h
#define dictu_path_h

#include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifdef _WIN32
#include <windows.h>
#define DIR_SEPARATOR '\\'
#define DIR_SEPARATOR_AS_STRING "\\"
#define DIR_SEPARATOR_STRLEN 1
#define PATH_DELIMITER ';'
#define PATH_DELIMITER_AS_STRING ";"
#define PATH_DELIMITER_STRLEN 1
#else
#include <dirent.h>

#define HAS_REALPATH
#define DIR_SEPARATOR '/'
#define DIR_SEPARATOR_AS_STRING "/"
#define DIR_SEPARATOR_STRLEN 1
#define PATH_DELIMITER ':'
#define PATH_DELIMITER_AS_STRING ":"
#define PATH_DELIMITER_STRLEN 1
#endif

#define IS_DIR_SEPARATOR(c) (c == DIR_SEPARATOR)

#include "optionals.h"
#include "../vm.h"
#include "../memory.h"

ObjModule *createPathClass(VM *vm);

#endif //dictu_path_h
