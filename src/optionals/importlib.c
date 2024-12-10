#include "importlib.h"

Value includeNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "include() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "include() argument must be a string");
        return EMPTY_VAL;
    }

    CallFrame *frame = &vm->frames[vm->frameCount - 1];
    char path[PATH_MAX];

    if (!resolvePath(frame->closure->function->module->path->chars, AS_CSTRING(args[0]), path)) {
        runtimeError(vm, "Could not open file \"%s\".", AS_CSTRING(args[0]));
        return EMPTY_VAL;
    }

    ObjString *pathObj = copyString(vm, path, strlen(path));
    push(vm, OBJ_VAL(pathObj));
    ObjModule *module = newModule(vm, pathObj);
    pop(vm);

    push(vm, OBJ_VAL(module));
    module->path = getDirectory(vm, path);

    char *source = readFile(vm, path);

    ObjFunction *function = compile(vm, module, source);
    push(vm, OBJ_VAL(function));
    FREE_ARRAY(vm, char, source, strlen(source) + 1);

    if (function == NULL) {
        return EMPTY_VAL;
    }

    ObjClosure *closure = newClosure(vm, function);
    pop(vm);

    callFunction(vm, OBJ_VAL(closure), 0, NULL);
    pop(vm);

    return OBJ_VAL(module);
}

Value createImportlibModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Importlib", 9);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    defineNative(vm, &module->values, "include", includeNative);

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
