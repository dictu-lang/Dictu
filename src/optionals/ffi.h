#ifndef dictu_ffi_module_h
#define dictu_ffi_module_h

#include <stdlib.h>

#include "optionals.h"
#include "../vm/vm.h"

#ifdef _WIN32
#define LIB_EXTENSION ".dll"
#define LIB_EXTENSION_STRLEN 4
#elif __APPLE__
#define LIB_EXTENSION ".dylib"
#define LIB_EXTENSION_STRLEN 6
#else
#define LIB_EXTENSION ".so"
#define LIB_EXTENSION_STRLEN 3
#endif

// This is used to determine if we can safely load the function pointers without UB,
// if this is greater then the version from the mod we error in the internal mod load function.
#define DICTU_FFI_API_VERSION 3


Value createFFIModule(DictuVM *vm);

#endif //dictu_ffi_module_h
