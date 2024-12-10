#include "importlib.h"

Value importNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(argCount);
    char path[PATH_MAX];

    CallFrame *frame = &vm->frames[vm->frameCount - 1];

    if (!resolvePath(frame->closure->function->module->path->chars, AS_CSTRING(args[0]), path)) {
        // ERROR HERE
    }

    ObjString *pathObj = copyString(vm, path, strlen(path));
    push(vm, OBJ_VAL(pathObj));

    char *source = readFile(vm, path);

//    ObjString *name = copyString(vm, moduleName, strlen(moduleName));
//    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, pathObj);
    pop(vm);
//    pop(vm);

    push(vm, OBJ_VAL(module));
    module->path = getDirectory(vm, path);
    pop(vm);

    ObjFunction *function = compile(vm, module, source);
    if (function == NULL) return INTERPRET_COMPILE_ERROR;
    push(vm, OBJ_VAL(function));
    ObjClosure *closure = newClosure(vm, function);
    pop(vm);

    FREE_ARRAY(vm, char, source, strlen(source) + 1);

    callFunction(vm, OBJ_VAL(closure), 0, NULL);
//    push(vm, OBJ_VAL(closure));
//    callValue(vm, OBJ_VAL(closure), 0, false);
//    DictuInterpretResult result = run(vm);

    return OBJ_VAL(module);
}

Value createImportlibModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Importlib", 9);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    defineNative(vm, &module->values, "include", importNative);

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
