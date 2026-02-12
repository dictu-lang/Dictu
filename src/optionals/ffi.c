#include "ffi.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

typedef struct {
#ifdef _WIN32
    HMODULE library;
#else
    void *library;
#endif
    char *path;
} FFIInstance;

typedef struct _vm_external vm_external;

typedef Value function_definition_t(DictuVM *vm, int argCount, Value *args);
typedef int init_func_definition_t(void **function_ptrs, DictuVM *vm,
                                   Table *table, int vm_ffi_version);


void *ffi_function_pointers[] = {&copyString,
                                 &newList,
                                 &newDict,
                                 &newSet,
                                 &newFile,
                                 &newAbstract,
                                 &newResult,
                                 &newResultSuccess,
                                 &newResultError,
                                 &push,
                                 &peek,
                                 &runtimeError,
                                 &pop,
                                 &isFalsey,
                                 &valuesEqual,
                                 &initValueArray,
                                 &writeValueArray,
                                 &freeValueArray,
                                 &dictSet,
                                 &dictGet,
                                 &dictDelete,
                                 &setGet,
                                 &setInsert,
                                 &setDelete,
                                 &valueToString,
                                 &valueTypeToString,
                                 &printValue,
                                 &printValueError,
                                 &compareStringLess,
                                 &compareStringGreater,
                                 &defineNative,
                                 &defineNativeProperty,
                                 &reallocate,
                                 &callFunction};

void freeFFI(DictuVM *vm, ObjAbstract *abstract) {
    FFIInstance *instance = (FFIInstance *)abstract->data;
    free(instance->path);
#ifdef _WIN32
    FreeLibrary(instance->library);
#else
    dlclose(instance->library);
#endif
    FREE(vm, FFIInstance, abstract->data);
}

char *ffiToString(ObjAbstract *abstract) {
    UNUSED(abstract);

    char *ffiString = malloc(sizeof(char) * 11 + 3);
    snprintf(ffiString, 11 + 3, "<FFIInstance>");
    return ffiString;
}

void grayFFI(DictuVM *vm, ObjAbstract *abstract) {
    (void)vm;
    FFIInstance *ffi = (FFIInstance *)abstract->data;

    if (ffi == NULL)
        return;
}

static Value load(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "load() takes one argument (%d given)", argCount);
        return EMPTY_VAL;
    }
    ObjString *path = AS_STRING(args[0]);
#ifdef _WIN32
    HMODULE library = LoadLibrary(path->chars);
#else
    void *library = dlopen(path->chars, RTLD_LAZY);
#endif
    if (!library) {
        runtimeError(vm, "Couldn't load shared object: %s", path->chars);
        return EMPTY_VAL;
    }
    ObjAbstract *abstract = newAbstractExcludeSelf(vm, freeFFI, ffiToString);
    push(vm, OBJ_VAL(abstract));
#ifdef _WIN32
    FARPROC init_func = GetProcAddress(library, "dictu_internal_ffi_init");
#else
    init_func_definition_t *init_func =
        dlsym(library, "dictu_internal_ffi_init");
#endif
    // call init function to give ffi module the required pointers to the
    // function of the vm.
    if (!init_func) {
        runtimeError(vm, "Couldn't initialize ffi api: %s", path->chars);
#ifdef _WIN32
        FreeLibrary(library);
#else
        dlclose(library);
#endif
        return EMPTY_VAL;
    }
    int initResult = init_func((void **)&ffi_function_pointers, vm,
                               &abstract->values, DICTU_FFI_API_VERSION);
    if (initResult == 1) {
        runtimeError(vm, "FFI mod already initialized: %s", path->chars);
#ifdef _WIN32
        FreeLibrary(library);
#else
        dlclose(library);
#endif
        return EMPTY_VAL;
    }
    if (initResult == 2) {
        runtimeError(vm,
                     "FFI api version is newer then mod version: %s, required "
                     "FFI version: %d",
                     path->chars, DICTU_FFI_API_VERSION);
#ifdef _WIN32
        FreeLibrary(library);
#else
        dlclose(library);
#endif
        return EMPTY_VAL;
    }
    if (initResult > 3) {
        runtimeError(vm,
                     "Mod init function returned a error: %s, error code: %d",
                     path->chars, initResult - 3);
#ifdef _WIN32
        FreeLibrary(library);
#else
        dlclose(library);
#endif
        return EMPTY_VAL;
    }
    FFIInstance *instance = ALLOCATE(vm, FFIInstance, 1);
    instance->path = malloc(path->length + 1);
    instance->path[path->length] = '\0';
    memcpy(instance->path, path->chars, path->length);

    instance->library = library;

    abstract->data = instance;
    abstract->grayFunc = grayFFI;
    pop(vm);

    return OBJ_VAL(abstract);
}

Value createFFIModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "FFI", 3);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));
    defineNative(vm, &module->values, "load", load);

    defineNativeProperty(
        vm, &module->values, "suffix",
        OBJ_VAL(copyString(vm, LIB_EXTENSION, LIB_EXTENSION_STRLEN)));
    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
