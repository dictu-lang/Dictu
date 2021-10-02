#include "path.h"

#if defined(_WIN32) && !defined(S_ISDIR)
#define S_ISDIR(M) (((M) & _S_IFDIR) == _S_IFDIR)
#endif

#ifdef HAS_REALPATH
static Value realpathNative(DictuVM *vm, int argCount, Value *args) {
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
        ERROR_RESULT;
    }

    return newResultSuccess(vm, OBJ_VAL(copyString(vm, tmp, strlen (tmp))));
}
#endif

static Value isAbsoluteNative(DictuVM *vm, int argCount, Value *args) {
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

static Value basenameNative(DictuVM *vm, int argCount, Value *args) {
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

    if (!len || (len == 1 && !IS_DIR_SEPARATOR(*path))) {
        return OBJ_VAL(copyString(vm, "", 0));
    }

    char *p = path + len - 1;
    while (p > path && !IS_DIR_SEPARATOR(*(p - 1))) --p;

    return OBJ_VAL(copyString(vm, p, (len - (p - path))));
}

static Value extnameNative(DictuVM *vm, int argCount, Value *args) {
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

static Value dirnameNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "dirname() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "dirname() argument must be a string");
        return EMPTY_VAL;
    }

    ObjString *PathString = AS_STRING(args[0]);
    return OBJ_VAL(dirname(vm, PathString->chars, PathString->length));
}

static Value existsNative(DictuVM *vm, int argCount, Value *args) {
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

static Value isdirNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "isDir() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "isDir() argument must be a string");
        return EMPTY_VAL;
    }

    char *path = AS_CSTRING(args[0]);
    struct stat path_stat;
    stat(path, &path_stat);

    if (S_ISDIR(path_stat.st_mode))
        return TRUE_VAL;

    return FALSE_VAL;

}

static Value listDirNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount > 1) {
        runtimeError(vm, "listDir() takes 0 or 1 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    char *path;
    if (argCount == 0) {
        path = ".";
    } else {
        if (!IS_STRING(args[0])) {
            runtimeError(vm, "listDir() argument must be a string");
            return EMPTY_VAL;
        }
        path = AS_CSTRING(args[0]);
    }

    ObjList *dir_contents = newList(vm);
    push(vm, OBJ_VAL(dir_contents));

    #ifdef _WIN32
    int length = strlen(path) + 4;
    char *searchPath = ALLOCATE(vm, char, length);
    if (searchPath == NULL) {
        runtimeError(vm, "Memory error on listDir()!");
        return EMPTY_VAL;
    }
    strcpy(searchPath, path);
    strcat(searchPath, "\\*");

    WIN32_FIND_DATAA file;
    HANDLE dir = FindFirstFile(searchPath, &file);
    if (dir == INVALID_HANDLE_VALUE) {
        runtimeError(vm, "%s is not a path!", path);
        free(searchPath);
        return EMPTY_VAL;
    }

    do {
        if (strcmp(file.cFileName, ".") == 0 || strcmp(file.cFileName, "..") == 0) {
            continue;
        }

        Value fileName = OBJ_VAL(copyString(vm, file.cFileName, strlen(file.cFileName)));
        push(vm, fileName);
        writeValueArray(vm, &dir_contents->values, fileName);
        pop(vm);
    } while (FindNextFile(dir, &file) != 0);

    FindClose(dir);
    FREE_ARRAY(vm, char, searchPath, length);
    #else
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

    closedir(d);
    #endif

    pop(vm);

    return OBJ_VAL(dir_contents);
}

static Value joinNative(DictuVM *vm, int argCount, Value *args) {
    char* argCountError = "join() requires 1 or more arguments (%d given).";
    char* nonStringError = "join() argument at index %d is not a string";

    if (argCount == 1 && IS_LIST(args[0])) {
        argCountError = "List passed to join() must have 1 or more elements (%d given).";
        nonStringError = "The element at index %d of the list passed to join() is not a string";
        ObjList *list = AS_LIST(args[0]);
        argCount = list->values.count;
        args = list->values.values;
    }

    if (argCount == 0) {
        runtimeError(vm, argCountError, argCount);
        return EMPTY_VAL;
    }

    for (int i = 0; i < argCount; ++i) {
        if (!IS_STRING(args[i])) {
            runtimeError(vm, nonStringError, i);
            return EMPTY_VAL;
        }
    }

    // resultSize = # of dir separators that will be used + length of each string arg
    size_t resultSize = abs(argCount - 1); // abs is needed her because of a clang bug
    for (int i = 0; i < argCount; ++i) resultSize += AS_STRING(args[i])->length;
    // It's possible for resultSize to be too large if the strings already end with the dir
    // separator, but having likely at most a few bytes of unused memory in the resulting string
    // object's char array should be fine.

    char* str = ALLOCATE(vm, char, resultSize);
    char* dest = str;
    for (int i = 0; i < argCount; ++i) {
        ObjString* src = AS_STRING(args[i]);
        // Append the src string to the end of dest
        for (int j = 0; j < src->length; ++j) *dest++ = src->chars[j];
        // Append a DIR_SEPARATOR if necessary
        if (src->chars[src->length-1] == DIR_SEPARATOR) --resultSize;
        else *dest++ = DIR_SEPARATOR;
    }

    return OBJ_VAL(takeString(vm, str, resultSize));
}

ObjModule *createPathModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Path", 4);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Path methods
     */
#ifdef HAS_REALPATH
    defineNative(vm, &module->values, "realpath", realpathNative);
#endif
    defineNative(vm, &module->values, "isAbsolute", isAbsoluteNative);
    defineNative(vm, &module->values, "basename", basenameNative);
    defineNative(vm, &module->values, "extname", extnameNative);
    defineNative(vm, &module->values, "dirname", dirnameNative);
    defineNative(vm, &module->values, "exists", existsNative);
    defineNative(vm, &module->values, "isDir", isdirNative);
    defineNative(vm, &module->values, "listDir", listDirNative);
    defineNative(vm, &module->values, "join", joinNative);

    /**
     * Define Path properties
     */
    defineNativeProperty(vm, &module->values, "delimiter", OBJ_VAL(
        copyString(vm, PATH_DELIMITER_AS_STRING, PATH_DELIMITER_STRLEN)));
    defineNativeProperty(vm, &module->values, "dirSeparator", OBJ_VAL(
        copyString(vm, DIR_SEPARATOR_AS_STRING, DIR_SEPARATOR_STRLEN)));
    pop(vm);
    pop(vm);

    return module;
}
