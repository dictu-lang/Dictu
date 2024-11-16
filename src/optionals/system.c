#include "system.h"
#include "../vm/utf8.h"

#ifdef _WIN32
#define rmdir(DIRNAME) _rmdir(DIRNAME)
#define chdir(DIRNAME) _chdir(DIRNAME)
#define chmod(FILENAME, MODE) _chmod(FILENAME, MODE)
#define getcwd(BUFFER, MAXLEN) _getcwd(BUFFER, MAXLEN)
#endif

#ifndef _WIN32
static Value getgidNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "getgid() doesn't take any argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    return NUMBER_VAL(getgid());
}

static Value getegidNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "getegid() doesn't take any argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    return NUMBER_VAL(getegid());
}

static Value getuidNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "getuid() doesn't take any argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    return NUMBER_VAL(getuid());
}

static Value geteuidNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "geteuid() doesn't take any argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    return NUMBER_VAL(geteuid());
}

static Value getppidNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "getppid() doesn't take any argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    return NUMBER_VAL(getppid());
}

static Value getpidNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "getpid() doesn't take any argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    return NUMBER_VAL(getpid());
}

static Value chownNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 3) {
        runtimeError(vm, "chown() takes 3 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "first chown() argument must be a string.");
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        runtimeError(vm, "second and third chown() arguments must be numbers.");
        return EMPTY_VAL;
    }

    ObjString *file = AS_STRING(args[0]);
    int uid = AS_NUMBER(args[1]);
    int gid = AS_NUMBER(args[2]);

    if (chown(file->chars, uid, gid) == -1) {
        ERROR_RESULT;
    }

    return newResultSuccess(vm, NIL_VAL);
}

static Value unameNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "uname() doesn't take any arguments (%d given)).", argCount);
        return EMPTY_VAL;
    }

    struct utsname u;
    if (uname(&u) == -1) {
        runtimeError(vm, "uname() failed to retrieve information");
        return EMPTY_VAL;
    }

    ObjDict *unameDict = newDict(vm);
    push(vm, OBJ_VAL(unameDict));

    ObjString *sysname = copyString(vm, "sysname", 7);
    push(vm, OBJ_VAL(sysname));
    ObjString *sysnameVal = copyString(vm, u.sysname, strlen(u.sysname));
    push(vm, OBJ_VAL(sysnameVal));
    dictSet(vm, unameDict, OBJ_VAL(sysname), OBJ_VAL(sysnameVal));
    pop(vm);
    pop(vm);

    ObjString *nodename = copyString(vm, "nodename", 8);
    push(vm, OBJ_VAL(nodename));
    ObjString *nodenameVal = copyString(vm, u.nodename, strlen(u.nodename));
    push(vm, OBJ_VAL(nodenameVal));
    dictSet(vm, unameDict, OBJ_VAL(nodename), OBJ_VAL(nodenameVal));
    pop(vm);
    pop(vm);

    ObjString *machine = copyString(vm, "machine", 7);
    push(vm, OBJ_VAL(machine));
    ObjString *machineVal = copyString(vm, u.machine, strlen(u.machine));
    push(vm, OBJ_VAL(machineVal));
    dictSet(vm, unameDict, OBJ_VAL(machine), OBJ_VAL(machineVal));
    pop(vm);
    pop(vm);

    ObjString *release = copyString(vm, "release", 7);
    push(vm, OBJ_VAL(release));
    ObjString *releaseVal = copyString(vm, u.release, strlen(u.release));
    push(vm, OBJ_VAL(releaseVal));
    dictSet(vm, unameDict, OBJ_VAL(release), OBJ_VAL(releaseVal));
    pop(vm);
    pop(vm);

    ObjString *version = copyString(vm, "version", 7);
    push(vm, OBJ_VAL(version));
    ObjString *versionVal = copyString(vm, u.version, strlen(u.version));
    push(vm, OBJ_VAL(versionVal));
    dictSet(vm, unameDict, OBJ_VAL(version), OBJ_VAL(versionVal));
    pop(vm);
    pop(vm);

    pop(vm);
    
    return OBJ_VAL(unameDict);
}

static Value mkdirTempNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount > 1) {
        runtimeError(vm, "mkdirTemp() takes 0 or 1 argument(s) (%d given)", argCount);
        return EMPTY_VAL;
    }

    char *template = "XXXXXX";

    if (argCount == 1) {
        if (!IS_STRING(args[0])) {
            runtimeError(vm, "mkdirTemp() first argument must be a string");
            return EMPTY_VAL;
        }

        template = AS_CSTRING(args[0]);
    }

    char *tmpl = {0};
    int size;

    if (template[0] != '\0') {
        size = strlen(template) + 1;
        tmpl = ALLOCATE(vm, char, size);
        strcpy(tmpl, template);
    } else {
        size = 7;
        tmpl = ALLOCATE(vm, char, size);
        strcpy(tmpl, "XXXXXX");
    }

    char *tmpDir = mkdtemp(tmpl);
    if (!tmpDir) {
        FREE_ARRAY(vm, char, tmpl, size);
        ERROR_RESULT;    
    }

    return newResultSuccess(vm, OBJ_VAL(takeString(vm, tmpDir, size - 1)));
}
#endif

static Value rmdirNative(DictuVM *vm, int argCount, Value *args) {
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

    if (retval < 0) {
        ERROR_RESULT;
    }

    return newResultSuccess(vm, NIL_VAL);
}

static Value mkdirNative(DictuVM *vm, int argCount, Value *args) {
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
    if (retval < 0) {
        ERROR_RESULT;
    }

    return newResultSuccess(vm, NIL_VAL);
}

static Value mkdirAllNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount == 0 || argCount > 2) {
        runtimeError(vm, "mkdirAll() takes 1 or 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "mkdirAll() first argument must be a string");
        return EMPTY_VAL;
    }

    ObjString *dir = AS_STRING(args[0]);

    int mode = 0777;

    if (argCount == 2) {
        if (!IS_NUMBER(args[1])) {
            runtimeError(vm, "mkdirAll() second argument must be a number");
            return EMPTY_VAL;
        }

        mode = AS_NUMBER(args[1]);
    }
    int retval;
    bool lastSeperator = false;
    int byteOffset = 0;
    char* ptr = dir->chars;
    for(int i = 0; i < dir->character_len; i++) {
        utf8_int32_t cp;
        char *n = utf8codepoint(ptr, &cp);
        if(cp == '/' || cp == '\\') {
            if(byteOffset > 0){
                dir->chars[byteOffset] = '\0';
                retval = MKDIR(dir->chars, mode);
                dir->chars[byteOffset] = cp;
            }
            lastSeperator = true;
        } else {
            lastSeperator = false;
        }
        ptr = n;
        byteOffset += utf8codepointsize(cp);
    }
    if(!lastSeperator){
        retval = MKDIR(dir->chars, mode);
    }
    
    if (retval < 0) {
        ERROR_RESULT;
    }

    return newResultSuccess(vm, NIL_VAL);
}

#ifdef HAS_ACCESS
static Value accessNative(DictuVM *vm, int argCount, Value *args) {
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

    int retval = access(file, mode);

    if (retval < 0) {
        ERROR_RESULT;
    }

    return newResultSuccess(vm, NIL_VAL);
}
#endif

static Value removeNative(DictuVM *vm, int argCount, Value *args) {
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

    if (retval < 0) {
        ERROR_RESULT;
    }

    return newResultSuccess(vm, NIL_VAL);
}

static Value setCWDNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "setCWD() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "setCWD() argument must be a string");
        return EMPTY_VAL;
    }

    char *dir = AS_CSTRING(args[0]);

    int retval = chdir(dir);

    if (retval < 0) {
        ERROR_RESULT;
    }

    return newResultSuccess(vm, NIL_VAL);
}

static Value getCWDNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(argCount); UNUSED(args);

    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        return newResultSuccess(vm, OBJ_VAL(copyString(vm, cwd, strlen(cwd))));
    }

    ERROR_RESULT;
}

static Value timeNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(vm); UNUSED(argCount); UNUSED(args);

    return NUMBER_VAL((double) time(NULL));
}

static Value clockNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(vm); UNUSED(argCount); UNUSED(args);

    return NUMBER_VAL((double) clock() / CLOCKS_PER_SEC);
}

static Value collectNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(argCount); UNUSED(args);

    collectGarbage(vm);
    return NIL_VAL;
}

static Value sleepNative(DictuVM *vm, int argCount, Value *args) {
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
#elif _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = stopTime;
    ts.tv_nsec = fmod(stopTime, 1) * 1000000000;
    nanosleep(&ts, NULL);
#else
    if (stopTime >= 1)
      sleep(stopTime);

    // 1000000 = 1 second
    usleep(fmod(stopTime, 1) * 1000000);
#endif

    return NIL_VAL;
}

static Value exitNative(DictuVM *vm, int argCount, Value *args) {
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

static Value chmodNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "chmod() takes 2 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0]) || !IS_STRING(args[1])) {
        runtimeError(vm, "chmod() arguments must be strings.");
        return EMPTY_VAL;
    }

    ObjString *file = AS_STRING(args[0]);
    ObjString *mode = AS_STRING(args[1]);

    int i = strtol(mode->chars, 0, 8);

    if (chmod(file->chars, i) == -1) {
        ERROR_RESULT;
    }

    return newResultSuccess(vm, NIL_VAL);
}

static Value renameNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "rename() takes 2 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0]) || !IS_STRING(args[1])) {
        runtimeError(vm, "rename() arguments must be strings.");
        return EMPTY_VAL;
    }

    char *old = AS_CSTRING(args[0]);
    char *new = AS_CSTRING(args[1]);

    if (rename(old, new) != 0) {
        return newResultError(vm, "failed to reaneme file");
    }

    return newResultSuccess(vm, NIL_VAL);
}

void initArgv(DictuVM *vm, Table *table, int argc, char **argv) {
    ObjList *list = newList(vm);
    push(vm, OBJ_VAL(list));

    for (int i = 0; i < argc; i++) {
        Value arg = OBJ_VAL(copyString(vm, argv[i], strlen(argv[i])));
        push(vm, arg);
        writeValueArray(vm, &list->values, arg);
        pop(vm);
    }

    defineNativeProperty(vm, table, "argv", OBJ_VAL(list));
    pop(vm);
}

void initPlatform(DictuVM *vm, Table *table) {
#ifdef _WIN32
    defineNativeProperty(vm, table, "platform", OBJ_VAL(copyString(vm, "windows", 7)));
#else
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

void setVersion(DictuVM *vm, Table *table) {
    ObjDict *versionDict = newDict(vm);
    push(vm, OBJ_VAL(versionDict));

    ObjString *major = copyString(vm, "major", 5);
    push(vm, OBJ_VAL(major));
    ObjString *majorVersion = copyString(vm, DICTU_MAJOR_VERSION, strlen(DICTU_MAJOR_VERSION));
    push(vm, OBJ_VAL(majorVersion));
    dictSet(vm, versionDict, OBJ_VAL(major), OBJ_VAL(majorVersion));
    pop(vm);
    pop(vm);

    ObjString *minor = copyString(vm, "minor", 5);
    push(vm, OBJ_VAL(minor));
    ObjString *minorVersion = copyString(vm, DICTU_MINOR_VERSION, strlen(DICTU_MINOR_VERSION));
    push(vm, OBJ_VAL(minorVersion));
    dictSet(vm, versionDict, OBJ_VAL(minor), OBJ_VAL(minorVersion));
    pop(vm);
    pop(vm);

    ObjString *patch = copyString(vm, "patch", 5);
    push(vm, OBJ_VAL(patch));
    ObjString *patchVersion = copyString(vm, DICTU_PATCH_VERSION, strlen(DICTU_PATCH_VERSION));
    push(vm, OBJ_VAL(patchVersion));
    dictSet(vm, versionDict, OBJ_VAL(patch), OBJ_VAL(patchVersion));
    pop(vm);
    pop(vm);

    defineNativeProperty(vm, table, "version", OBJ_VAL(versionDict));

    pop(vm);
}

Value createSystemModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "System", 6);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define System methods
     */
#ifndef _WIN32
    defineNative(vm, &module->values, "getgid", getgidNative);
    defineNative(vm, &module->values, "getegid", getegidNative);
    defineNative(vm, &module->values, "getuid", getuidNative);
    defineNative(vm, &module->values, "geteuid", geteuidNative);
    defineNative(vm, &module->values, "getppid", getppidNative);
    defineNative(vm, &module->values, "getpid", getpidNative);
    defineNative(vm, &module->values, "chown", chownNative);
    defineNative(vm, &module->values, "uname", unameNative);
    defineNative(vm, &module->values, "mkdirTemp", mkdirTempNative);
#endif
    defineNative(vm, &module->values, "rmdir", rmdirNative);
    defineNative(vm, &module->values, "mkdir", mkdirNative);
    defineNative(vm, &module->values, "mkdirAll", mkdirAllNative);
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
    defineNative(vm, &module->values, "chmod", chmodNative);
    defineNative(vm, &module->values, "rename", renameNative);

    /**
     * Define System properties
     */
    if (!vm->repl) {
        // Set argv variable
        initArgv(vm, &module->values, vm->argc, vm->argv);
    }

    initPlatform(vm, &module->values);
    setVersion(vm, &module->values);

    defineNativeProperty(vm, &module->values, "arch", OBJ_VAL(copyString(vm, SYSTEM_ARCH, strlen(SYSTEM_ARCH))));

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

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
