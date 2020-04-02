#include "path.h"

static Value isAbsoluteNative(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "isAbsolute() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "isAbsolute() argument must be a string");
        return EMPTY_VAL;
    }

    char *path = AS_CSTRING(args[0]);

    return (IS_DIR_SEPARATOR(path[0]) ? TRUE_VAL : FALSE_VAL);
}

static Value basenameNative(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "basename() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "basename() argument must be a string");
        return EMPTY_VAL;
    }

    ObjString *PathString = AS_STRING(args[0]);
    char *path = PathString->chars;

    int len = PathString->length;

    if (!len || (len == 1 && *path != DIR_SEPARATOR)) {
        ObjString *newPath = copyString(vm, "", 0);
        return OBJ_VAL(newPath);
    }

    char *p = path + len - 1;
    while (p > path && (*(p - 1) != DIR_SEPARATOR)) --p;

    ObjString *newPath = copyString(vm, p, (len - (p - path)));
    return OBJ_VAL(newPath);
}

static Value extnameNative(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "extname() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "extname() argument must be a string");
        return EMPTY_VAL;
    }

    ObjString *PathString = AS_STRING(args[0]);
    char *path = PathString->chars;

    int len = PathString->length;

    if (!len) {
        ObjString *newPath = copyString(vm, path, len);
        return OBJ_VAL(newPath);
    }

    char *p = path + len;
    while (p > path && (*(p - 1) != '.')) --p;

    if (p == path) {
        ObjString *newPath = copyString(vm, "", 0);
        return OBJ_VAL(newPath);
    }

    p--;

    ObjString *newPath = copyString(vm, p, len - (p - path));
    return OBJ_VAL(newPath);
}

static Value dirnameNative(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "dirname() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "dirname() argument must be a string");
        return EMPTY_VAL;
    }

    ObjString *PathString = AS_STRING(args[0]);
    char *path = PathString->chars;

    int len = PathString->length;

    if (!len) {
        ObjString *newPath = copyString(vm, ".", 1);
        return OBJ_VAL(newPath);
    }

    char *sep = path + len;

    /* trailing slashes */
    while (sep != path) {
        if (0 == IS_DIR_SEPARATOR (*sep))
            break;
        sep--;
    }

    /* first found */
    while (sep != path) {
        if (IS_DIR_SEPARATOR (*sep))
            break;
        sep--;
    }

    /* trim again */
    while (sep != path) {
        if (0 == IS_DIR_SEPARATOR (*sep))
            break;
        sep--;
    }

    if (sep == path && *sep != DIR_SEPARATOR) {
        ObjString *newPath = copyString(vm, ".", 1);
        return OBJ_VAL(newPath);
    }

    len = sep - path + 1;

    ObjString *newPath = copyString(vm, path, len);
    return OBJ_VAL(newPath);
}

void createPathClass(VM *vm) {
    ObjString *name = copyString(vm, "Path", 4);
    push(vm, OBJ_VAL(name));
    ObjClassNative *klass = newClassNative(vm, name);
    push(vm, OBJ_VAL(klass));

    /**
     * Define Path methods
     */
    defineNative(vm, &klass->methods, "isAbsolute", isAbsoluteNative);
    defineNative(vm, &klass->methods, "basename", basenameNative);
    defineNative(vm, &klass->methods, "extname", extnameNative);
    defineNative(vm, &klass->methods, "dirname", dirnameNative);
    tableSet(vm, &vm->globals, name, OBJ_VAL(klass));
    pop(vm);
    pop(vm);
}
