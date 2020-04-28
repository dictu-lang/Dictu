#include "system.h"

static Value rmdirNative(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "rmdir() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "rmdir() argument must be a string");
        return EMPTY_VAL;
    }

    char *dir = AS_CSTRING(args[0]);

    int retval = rmdir(dir);

    return NUMBER_VAL(retval == 0 ? OK : NOTOK);
}

static Value mkdirNative(VM *vm, int argCount, Value *args) {
    if (argCount == 0 || argCount > 2) {
        runtimeError(vm, "mkdir() takes 1 or 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "mkdir() first argument must be a string");
        return EMPTY_VAL;
    }

    char *dir = AS_CSTRING(args[0]);

    int mode = 0777;

    if (argCount == 2) {
        if (!IS_NUMBER(args[1])) {
            runtimeError(vm, "mkdir() second argument must be a number");
            return EMPTY_VAL;
        }

        mode = AS_NUMBER(args[1]);
    }

    int retval = MKDIR(dir, mode);

    return NUMBER_VAL(retval == 0 ? OK : NOTOK);
}

static Value removeNative(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "remove() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "remove() argument must be a string");
        return EMPTY_VAL;
    }

    char *file = AS_CSTRING(args[0]);

    int retval = REMOVE(file);

    return NUMBER_VAL(retval == 0 ? OK : NOTOK);
}

static Value setCWDNative(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "setcwd() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "setcwd() argument must be a string");
        return EMPTY_VAL;
    }

    char *dir = AS_CSTRING(args[0]);

    int retval = chdir(dir);

    return NUMBER_VAL(retval == 0 ? OK : NOTOK);
}

static Value getCWDNative(VM *vm, int argCount, Value *args) {
    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        return OBJ_VAL(copyString(vm, cwd, strlen(cwd)));
    }

    runtimeError(vm, "Error getting current directory");
    return EMPTY_VAL;
}

static Value timeNative(VM *vm, int argCount, Value *args) {
    return NUMBER_VAL((double) time(NULL));
}

static Value clockNative(VM *vm, int argCount, Value *args) {
    return NUMBER_VAL((double) clock() / CLOCKS_PER_SEC);
}

static Value collectNative(VM *vm, int argCount, Value *args) {
    collectGarbage(vm);
    return NIL_VAL;
}

static Value sleepNative(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "sleep() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError(vm, "sleep() argument must be a number");
        return EMPTY_VAL;
    }

    double stopTime = AS_NUMBER(args[0]);

#ifdef _WIN32
    Sleep(stopTime * 1000);
#else
    sleep(stopTime);
#endif
    return NIL_VAL;
}

static Value exitNative(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "exit() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError(vm, "sleep() argument must be a number");
        return EMPTY_VAL;
    }

    exit(AS_NUMBER(args[0]));
    return EMPTY_VAL; /* satisfy the tcc compiler */

}

void initArgv(VM *vm, Table *table, int argc, const char *argv[]) {
    ObjList *list = initList(vm);
    push(vm, OBJ_VAL(list));

    for (int i = 1; i < argc; i++) {
        Value arg = OBJ_VAL(copyString(vm, argv[i], strlen(argv[i])));
        push(vm, arg);
        writeValueArray(vm, &list->values, arg);
        pop(vm);
    }

    defineNativeProperty(vm, table, "argv", OBJ_VAL(list));
    pop(vm);
}

void initPlatform(VM *vm, Table *table) {
#ifdef _WIN32
    defineNativeProperty(vm, table, "platform", OBJ_VAL(copyString(vm, "windows", 7)));
    return;
#endif

    struct utsname u;
    if (-1 == uname(&u)) {
        defineNativeProperty(vm, table, "platform", OBJ_VAL(copyString(vm,
            "unknown", 7)));
        return;
    }

    u.sysname[0] = tolower(u.sysname[0]);
    defineNativeProperty(vm, table, "platform", OBJ_VAL(copyString(vm, u.sysname,
        strlen(u.sysname))));
}

void createSystemClass(VM *vm, int argc, const char *argv[]) {
    ObjString *name = copyString(vm, "System", 6);
    push(vm, OBJ_VAL(name));
    ObjClassNative *klass = newClassNative(vm, name);
    push(vm, OBJ_VAL(klass));

    /**
     * Define System methods
     */
    defineNative(vm, &klass->methods, "rmdir", rmdirNative);
    defineNative(vm, &klass->methods, "mkdir", mkdirNative);
    defineNative(vm, &klass->methods, "remove", removeNative);
    defineNative(vm, &klass->methods, "setCWD", setCWDNative);
    defineNative(vm, &klass->methods, "getCWD", getCWDNative);
    defineNative(vm, &klass->methods, "time", timeNative);
    defineNative(vm, &klass->methods, "clock", clockNative);
    defineNative(vm, &klass->methods, "collect", collectNative);
    defineNative(vm, &klass->methods, "sleep", sleepNative);
    defineNative(vm, &klass->methods, "exit", exitNative);

    /**
     * Define System properties
     */
    if (!vm->repl) {
        // Set argv variable
        initArgv(vm, &klass->properties, argc, argv);
    }

    initPlatform(vm, &klass->properties);

    defineNativeProperty(vm, &klass->properties, "S_IRWXU", NUMBER_VAL(448));
    defineNativeProperty(vm, &klass->properties, "S_IRUSR", NUMBER_VAL(256));
    defineNativeProperty(vm, &klass->properties, "S_IWUSR", NUMBER_VAL(128));
    defineNativeProperty(vm, &klass->properties, "S_IXUSR", NUMBER_VAL(64));
    defineNativeProperty(vm, &klass->properties, "S_IRWXG", NUMBER_VAL(56));
    defineNativeProperty(vm, &klass->properties, "S_IRGRP", NUMBER_VAL(32));
    defineNativeProperty(vm, &klass->properties, "S_IWGRP", NUMBER_VAL(16));
    defineNativeProperty(vm, &klass->properties, "S_IXGRP", NUMBER_VAL(8));
    defineNativeProperty(vm, &klass->properties, "S_IRWXO", NUMBER_VAL(7));
    defineNativeProperty(vm, &klass->properties, "S_IROTH", NUMBER_VAL(4));
    defineNativeProperty(vm, &klass->properties, "S_IWOTH", NUMBER_VAL(2));
    defineNativeProperty(vm, &klass->properties, "S_IXOTH", NUMBER_VAL(1));
    defineNativeProperty(vm, &klass->properties, "S_ISUID", NUMBER_VAL(2048));
    defineNativeProperty(vm, &klass->properties, "S_ISGID", NUMBER_VAL(1024));

    tableSet(vm, &vm->globals, name, OBJ_VAL(klass));
    pop(vm);
    pop(vm);
}
