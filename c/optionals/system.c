#include "system.h"

#ifdef _WIN32
#define rmdir(DIRNAME) _rmdir(DIRNAME)
#define chdir(DIRNAME) _chdir(DIRNAME)
#define getcwd(BUFFER, MAXLEN) _getcwd(BUFFER, MAXLEN)
#endif

#ifndef _WIN32
static Value getgidNative(VM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "getgid() doesn't take any argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    return NUMBER_VAL(getgid());
}

static Value getegidNative(VM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "getegid() doesn't take any argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    return NUMBER_VAL(getegid());
}

static Value getuidNative(VM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "getuid() doesn't take any argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    return NUMBER_VAL(getuid());
}

static Value geteuidNative(VM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "geteuid() doesn't take any argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    return NUMBER_VAL(geteuid());
}

static Value getppidNative(VM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "getppid() doesn't take any argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    return NUMBER_VAL(getppid());
}

static Value getpidNative(VM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "getpid() doesn't take any argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    return NUMBER_VAL(getpid());
}
#endif

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

    if (-1 == retval) {
      SET_ERRNO(GET_SELF_CLASS);
    }

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

    if (retval == NOTOK) {
      SET_ERRNO(GET_SELF_CLASS);
    }

    return NUMBER_VAL(retval == 0 ? OK : NOTOK);
}

#ifdef HAS_ACCESS
static Value accessNative(VM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "access() takes 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "access() first argument must be a string");
        return EMPTY_VAL;
    }

    char *file = AS_CSTRING(args[0]);

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm, "access() second argument must be a number");
        return EMPTY_VAL;
    }

    int mode = AS_NUMBER(args[1]);

    RESET_ERRNO(GET_SELF_CLASS);

    int retval = access(file, mode);

    if (retval == -1) {
      SET_ERRNO(GET_SELF_CLASS);
    }

    return NUMBER_VAL((retval == -1 ? NOTOK : OK));
}
#endif

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

    if (retval == NOTOK) {
      SET_ERRNO(GET_SELF_CLASS);
    }

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

    if (retval == NOTOK) {
        SET_ERRNO(GET_SELF_CLASS);
    }

    return NUMBER_VAL(retval == 0 ? OK : NOTOK);
}

static Value getCWDNative(VM *vm, int argCount, Value *args) {
    UNUSED(argCount); UNUSED(args);

    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        return OBJ_VAL(copyString(vm, cwd, strlen(cwd)));
    }

    SET_ERRNO(GET_SELF_CLASS);

    return NIL_VAL;
}

static Value timeNative(VM *vm, int argCount, Value *args) {
    UNUSED(vm); UNUSED(argCount); UNUSED(args);

    return NUMBER_VAL((double) time(NULL));
}

static Value clockNative(VM *vm, int argCount, Value *args) {
    UNUSED(vm); UNUSED(argCount); UNUSED(args);

    return NUMBER_VAL((double) clock() / CLOCKS_PER_SEC);
}

static Value collectNative(VM *vm, int argCount, Value *args) {
    UNUSED(argCount); UNUSED(args);

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
        runtimeError(vm, "exit() argument must be a number");
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
#elif
    struct utsname u;
    if (-1 == uname(&u)) {
        defineNativeProperty(vm, table, "platform", OBJ_VAL(copyString(vm,
            "unknown", 7)));
        return;
    }

    u.sysname[0] = tolower(u.sysname[0]);
    defineNativeProperty(vm, table, "platform", OBJ_VAL(copyString(vm, u.sysname,
        strlen(u.sysname))));
#endif
}

void createSystemClass(VM *vm, int argc, const char *argv[]) {
    ObjString *name = copyString(vm, "System", 6);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define System methods
     */
    defineNative(vm, &module->values, "strerror", strerrorNative);
#ifndef _WIN32
    defineNative(vm, &module->values, "getgid", getgidNative);
    defineNative(vm, &module->values, "getegid", getegidNative);
    defineNative(vm, &module->values, "getuid", getuidNative);
    defineNative(vm, &module->values, "geteuid", geteuidNative);
    defineNative(vm, &module->values, "getppid", getppidNative);
    defineNative(vm, &module->values, "getpid", getpidNative);
#endif
    defineNative(vm, &module->values, "rmdir", rmdirNative);
    defineNative(vm, &module->values, "mkdir", mkdirNative);
#ifdef HAS_ACCESS
    defineNative(vm, &module->values, "access", accessNative);
#endif
    defineNative(vm, &module->values, "remove", removeNative);
    defineNative(vm, &module->values, "setCWD", setCWDNative);
    defineNative(vm, &module->values, "getCWD", getCWDNative);
    defineNative(vm, &module->values, "time", timeNative);
    defineNative(vm, &module->values, "clock", clockNative);
    defineNative(vm, &module->values, "collect", collectNative);
    defineNative(vm, &module->values, "sleep", sleepNative);
    defineNative(vm, &module->values, "exit", exitNative);

    /**
     * Define System properties
     */
    if (!vm->repl) {
        // Set argv variable
        initArgv(vm, &module->values, argc, argv);
    }

    initPlatform(vm, &module->values);

    defineNativeProperty(vm, &module->values, "errno", NUMBER_VAL(0));

    defineNativeProperty(vm, &module->values, "S_IRWXU", NUMBER_VAL(448));
    defineNativeProperty(vm, &module->values, "S_IRUSR", NUMBER_VAL(256));
    defineNativeProperty(vm, &module->values, "S_IWUSR", NUMBER_VAL(128));
    defineNativeProperty(vm, &module->values, "S_IXUSR", NUMBER_VAL(64));
    defineNativeProperty(vm, &module->values, "S_IRWXG", NUMBER_VAL(56));
    defineNativeProperty(vm, &module->values, "S_IRGRP", NUMBER_VAL(32));
    defineNativeProperty(vm, &module->values, "S_IWGRP", NUMBER_VAL(16));
    defineNativeProperty(vm, &module->values, "S_IXGRP", NUMBER_VAL(8));
    defineNativeProperty(vm, &module->values, "S_IRWXO", NUMBER_VAL(7));
    defineNativeProperty(vm, &module->values, "S_IROTH", NUMBER_VAL(4));
    defineNativeProperty(vm, &module->values, "S_IWOTH", NUMBER_VAL(2));
    defineNativeProperty(vm, &module->values, "S_IXOTH", NUMBER_VAL(1));
    defineNativeProperty(vm, &module->values, "S_ISUID", NUMBER_VAL(2048));
    defineNativeProperty(vm, &module->values, "S_ISGID", NUMBER_VAL(1024));
#ifdef HAS_ACCESS
    defineNativeProperty(vm, &module->values, "F_OK", NUMBER_VAL(F_OK));
    defineNativeProperty(vm, &module->values, "X_OK", NUMBER_VAL(X_OK));
    defineNativeProperty(vm, &module->values, "W_OK", NUMBER_VAL(W_OK));
    defineNativeProperty(vm, &module->values, "R_OK", NUMBER_VAL(R_OK));
#endif

    tableSet(vm, &vm->globals, name, OBJ_VAL(module));
    pop(vm);
    pop(vm);
}
