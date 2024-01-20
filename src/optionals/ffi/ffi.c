#include "ffi.h"
#include <dlfcn.h>


typedef struct {
    void* library;
    char* path;
    int size;
    char** names;
} FFIInstance;

typedef struct _vm_external vm_external;

typedef Value function_definition_t(DictuVM *vm, int argCount, Value *args);

#define AS_FFI_INSTANCE(v) ((FFIInstance*)AS_ABSTRACT(v)->data)


void freeFFI(DictuVM *vm, ObjAbstract *abstract) {
    FFIInstance *instance = (FFIInstance*)abstract->data;
    free(instance->path);
    if(instance->size > 0){
        for(int i = 0; i < instance->size; i++){
            if(instance->names[i] != NULL)
                free(instance->names[i]);
        }
        free(instance->names);
    }
    dlclose(instance->library);
    FREE(vm, FFIInstance, abstract->data);
}

char *ffiToString(ObjAbstract *abstract) {
    UNUSED(abstract);

    char *ffiString = malloc(sizeof(char) * 11+3);
    snprintf(ffiString, 11+3, "<FFIInstance>");
    return ffiString;
}

void grayFFI(DictuVM *vm, ObjAbstract *abstract) {
    (void)vm;
    FFIInstance *ffi = (FFIInstance*)abstract->data;

    if (ffi == NULL) return;

}
static Value ffiInstanceInvoke(DictuVM *vm, int argCount, Value *args) {
    if(argCount < 1){
        runtimeError(vm, "invoke() takes at least one argument (%d given)", argCount-1);
        return EMPTY_VAL;
    }
    FFIInstance* instance = AS_FFI_INSTANCE(args[0]);
    ObjString* symbol = AS_STRING(args[1]);
    bool found = false;
    for(int i = 0; i < instance->size; i++) {
        if(instance->names[i]) {
            if(strcmp(instance->names[i], symbol->chars) == 0){
                found = true;
                break;
            }
        }
    }
    if(!found){
        runtimeError(vm, "symbol not found: %s", symbol->chars);
        return EMPTY_VAL;
    }
    function_definition_t* ptr = dlsym(instance->library, symbol->chars);
    Value res = EMPTY_VAL;
    if(argCount == 1)
        res = ptr(vm, 0, NULL);
    else
        res = ptr(vm, argCount-1, args+2);
    return res;
}

static Value load(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "load() takes 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }
    ObjString* path = AS_STRING(args[0]);
    void *library = dlopen(path->chars, RTLD_LAZY);
    if(!library) {
        runtimeError(vm, "Couldn't load shared object: %s", path->chars);
        return EMPTY_VAL;
    }
    ObjList* symbols = AS_LIST(args[1]);
    ObjAbstract *abstract = newAbstract(vm, freeFFI, ffiToString);
    push(vm, OBJ_VAL(abstract));
    FFIInstance *instance = ALLOCATE(vm, FFIInstance, 1);
    instance->path = malloc(path->length+1);
    instance->path[path->length] = '\0';
    memcpy(instance->path, path->chars, path->length);

    instance->library = library;
    instance->size = symbols->values.count;
    if(instance->size > 0) {
        instance->names = malloc(sizeof(char*) * instance->size);
        for(int i = 0; i< symbols->values.count; i+= 1){
            Value v = symbols->values.values[i];
            if(!IS_STRING(v)) {
                instance->names[i] = NULL;
            } else {
                ObjString* sym = AS_STRING(v);
                if(dlsym(instance->library, sym->chars) == NULL){
                    runtimeError(vm, "Couldn't load symbol: %s", sym->chars);
                    return EMPTY_VAL;
                }
                instance->names[i] = malloc(sym->length+1);
                instance->names[i][sym->length] = '\0';
                memcpy(instance->names[i], sym->chars, sym->length);
            }
        }
    } else {
        instance->names = NULL;
    }

    defineNative(vm, &abstract->values, "invoke", ffiInstanceInvoke);

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
    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}