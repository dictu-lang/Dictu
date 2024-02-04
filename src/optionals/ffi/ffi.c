#include "ffi.h"
#include <dlfcn.h>

typedef struct {
    void *library;
    char *path;
} FFIInstance;

typedef struct _vm_external vm_external;

typedef Value function_definition_t(DictuVM *vm, int argCount, Value *args);
typedef int init_func_definition_t(void **function_ptrs);

// #define AS_FFI_INSTANCE(v) ((FFIInstance *)AS_ABSTRACT(v)->data)

void *ffi_function_pointers[] = {
    &copyString,
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
    &compareStringGreater
};

void freeFFI(DictuVM *vm, ObjAbstract *abstract) {
    FFIInstance *instance = (FFIInstance *)abstract->data;
    free(instance->path);
    dlclose(instance->library);
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
    if (argCount != 2) {
        runtimeError(vm, "load() takes 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }
    ObjString *path = AS_STRING(args[0]);
    void *library = dlopen(path->chars, RTLD_LAZY);
    if (!library) {
        runtimeError(vm, "Couldn't load shared object: %s", path->chars);
        return EMPTY_VAL;
    }
    init_func_definition_t *init_func =
        dlsym(library, "dictu_internal_ffi_init");
    // call init function to give ffi module the required pointers to the function of the vm.
    if (!init_func || init_func((void**)&ffi_function_pointers) != 0) {
        runtimeError(vm, "Couldn't initialize ffi api: %s", path->chars);
        dlclose(library);
        return EMPTY_VAL;
    }
    ObjList *symbols = AS_LIST(args[1]);
    ObjAbstract *abstract = newAbstractExcludeSelf(vm, freeFFI, ffiToString);
    push(vm, OBJ_VAL(abstract));
    FFIInstance *instance = ALLOCATE(vm, FFIInstance, 1);
    instance->path = malloc(path->length + 1);
    instance->path[path->length] = '\0';
    memcpy(instance->path, path->chars, path->length);

    instance->library = library;
    if (symbols->values.count) {
        for (int i = 0; i < symbols->values.count; i += 1) {
            Value v = symbols->values.values[i];
            if (IS_STRING(v))  {
                ObjString *sym = AS_STRING(v);
                void* funcptr = dlsym(instance->library, sym->chars);
                if(!funcptr) {
                    runtimeError(vm, "Couldn't find symbol: %s in %s", sym->chars, path->chars);
                    dlclose(library);
                    return EMPTY_VAL;
                }
                defineNative(vm, &abstract->values, sym->chars, funcptr);
            }
        }
    } 

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