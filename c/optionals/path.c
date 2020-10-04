#include "path.h"

#ifdef HAS_REALPATH
static Value realpathNative(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "realpath() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "realpath() argument must be a string");
        return EMPTY_VAL;
    }

    char *path = AS_CSTRING(args[0]);

    char tmp[PATH_MAX + 1];
    if (NULL == realpath(path, tmp)) {
        SET_ERRNO(GET_SELF_CLASS);
        return NIL_VAL;
    }

    return OBJ_VAL(copyString(vm, tmp, strlen (tmp)));
}
#endif

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

static Value isdirNative(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "isdir() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }
    
    if (!IS_STRING(args[0])) {
        runtimeError(vm, "isdir() argument must be a string");
        return EMPTY_VAL;
    }

    char *path = AS_CSTRING(args[0]);
    struct stat path_stat;
    stat(path, &path_stat);

    if (S_ISDIR(path_stat.st_mode))
        return TRUE_VAL;
   
    return FALSE_VAL;

}

static Value listdirNative(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "listdir() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "listdir() argument must be a string");
        return EMPTY_VAL;
    }

    ObjList *dir_contents = initList(vm);
    push(vm, OBJ_VAL(dir_contents));
    char *path = AS_CSTRING(args[0]);
    struct dirent *dir;
    DIR *d;
    d = opendir(path);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            char *inode_name = dir->d_name;
            if (strcmp(inode_name, ".") == 0 || strcmp(inode_name, "..") == 0) 
                continue;
            Value inode_value = OBJ_VAL(copyString(vm, inode_name, strlen(inode_name)));
            push(vm, inode_value);
            writeValueArray(vm, &dir_contents->values, inode_value);
            pop(vm);
        }
    } else {
        runtimeError(vm, "%s is not a path!", path);
        return EMPTY_VAL;
    }

    pop(vm);

    return OBJ_VAL(dir_contents);
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
        return OBJ_VAL(copyString(vm, "", 0));
    }

    char *p = path + len - 1;
    while (p > path && (*(p - 1) != DIR_SEPARATOR)) --p;

    return OBJ_VAL(copyString(vm, p, (len - (p - path))));
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
        return OBJ_VAL(copyString(vm, path, len));
    }

    char *p = path + len;
    while (p > path && (*(p - 1) != '.')) --p;

    if (p == path) {
        return OBJ_VAL(copyString(vm, "", 0));
    }

    p--;

    return OBJ_VAL(copyString(vm, p, len - (p - path)));
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
        return OBJ_VAL(copyString(vm, ".", 1));
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
        return OBJ_VAL(copyString(vm, ".", 1));
    }

    len = sep - path + 1;

    return OBJ_VAL(copyString(vm, path, len));
}

static Value existsNative(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "exists() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "exists() argument must be a string");
        return EMPTY_VAL;
    }

    char *path = AS_CSTRING(args[0]);

    struct stat buffer;

    return BOOL_VAL(stat(path, &buffer) == 0);
}

ObjModule *createPathClass(VM *vm) {
    ObjString *name = copyString(vm, "Path", 4);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Path methods
     */
#ifdef HAS_REALPATH
    defineNative(vm, &module->values, "realpath", realpathNative);
    defineNativeProperty(vm, &module->values, "errno", NUMBER_VAL(0));
    defineNative(vm, &module->values, "strerror", strerrorNative); // only realpath uses errno
#endif
    defineNative(vm, &module->values, "isAbsolute", isAbsoluteNative);
    defineNative(vm, &module->values, "basename", basenameNative);
    defineNative(vm, &module->values, "extname", extnameNative);
    defineNative(vm, &module->values, "dirname", dirnameNative);
    defineNative(vm, &module->values, "exists", existsNative);
    defineNative(vm, &module->values, "isdir", isdirNative);
    defineNative(vm, &module->values, "listdir", listdirNative);

    defineNativeProperty(vm, &module->values, "delimiter", OBJ_VAL(
        copyString(vm, PATH_DELIMITER_AS_STRING, PATH_DELIMITER_STRLEN)));
    defineNativeProperty(vm, &module->values, "dirSeparator", OBJ_VAL(
        copyString(vm, DIR_SEPARATOR_AS_STRING, DIR_SEPARATOR_STRLEN)));
    pop(vm);
    pop(vm);

    return module;
}
