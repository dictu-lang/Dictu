#ifndef dictu_include_h
#define dictu_include_h

#include <stdbool.h>

#define DICTU_MAJOR_VERSION "0"
#define DICTU_MINOR_VERSION "30"
#define DICTU_PATCH_VERSION "0"

#ifdef DEBUG

#define DICTU_DEBUG_VERSION_STR "(Debug)"

#else

#define DICTU_DEBUG_VERSION_STR ""

#endif

#if defined(GIT_HASH)

#define DICTU_STRING_VERSION DICTU_DEBUG_VERSION_STR "Dictu Version: "  DICTU_MAJOR_VERSION "." DICTU_MINOR_VERSION "." DICTU_PATCH_VERSION " [rev: " GIT_HASH "]\n"

#else

#define DICTU_STRING_VERSION DICTU_DEBUG_VERSION_STR "Dictu Version: " DICTU_MAJOR_VERSION "." DICTU_MINOR_VERSION "." DICTU_PATCH_VERSION "\n"

#endif

typedef struct _vm DictuVM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} DictuInterpretResult;

DictuVM *dictuInitVM(bool repl, int argc, char *argv[]);

void dictuFreeVM(DictuVM *vm);

DictuInterpretResult dictuInterpret(DictuVM *vm, char *moduleName, char *source);

#endif //dictu_include_h
