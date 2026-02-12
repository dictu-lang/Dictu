#include "importlib.h"

Value includeNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1 && argCount != 2) {
        runtimeError(vm, "include() takes 1 or 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "include() first argument must be a string");
        return EMPTY_VAL;
    }

    bool reload = false;

    if (argCount == 2) {
        if (!IS_BOOL(args[1])) {
            runtimeError(vm, "include() second argument must be a boolean");
            return EMPTY_VAL;
        }

        reload = AS_BOOL(args[1]);
    }

    CallFrame *frame = &vm->frames[vm->frameCount - 1];
    char path[PATH_MAX];

    if (!resolvePath(frame->closure->function->module->path->chars, AS_CSTRING(args[0]), path)) {
        runtimeError(vm, "Could not open file \"%s\".", AS_CSTRING(args[0]));
        return EMPTY_VAL;
    }

    Value moduleVal;
    ObjString *pathObj = copyString(vm, path, strlen(path));
    if (!reload && tableGet(&vm->modules, pathObj, &moduleVal)) {
        return moduleVal;
    }

    push(vm, OBJ_VAL(pathObj));
    ObjModule *module = newModule(vm, pathObj);
    pop(vm);

    push(vm, OBJ_VAL(module));
    module->path = dirname(vm, path, strlen(path));

    char *source = readFile(vm, path);

    if (source == NULL) {
        pop(vm); // module
        runtimeError(vm, "Could not open file \"%s\".", AS_CSTRING(args[0]));
        return EMPTY_VAL;
    }

    ObjFunction *function = compile(vm, module, source);
    FREE_ARRAY(vm, char, source, strlen(source) + 1);

    if (function == NULL) {
        pop(vm); // module
        runtimeError(vm, "Could not compile file \"%s\".", AS_CSTRING(args[0]));
        return EMPTY_VAL;
    }

    push(vm, OBJ_VAL(function));
    ObjClosure *closure = newClosure(vm, function);
    pop(vm); // function

    callFunction(vm, OBJ_VAL(closure), 0, NULL);
    pop(vm); // module

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
