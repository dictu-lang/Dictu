#include "system.h"

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

    if (retval != 0) {
        runtimeError(vm, "Error setting current directory");
        return EMPTY_VAL;
    }

    return NIL_VAL;
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

void createSystemClass(VM *vm) {
    ObjString *name = copyString(vm, "System", 6);
    push(vm, OBJ_VAL(name));
    ObjClassNative *klass = newClassNative(vm, name);
    push(vm, OBJ_VAL(klass));

    /**
     * Define System methods
     */
    defineNative(vm, &klass->methods, "setCWD", setCWDNative);
    defineNative(vm, &klass->methods, "getCWD", getCWDNative);
    defineNative(vm, &klass->methods, "time", timeNative);
    defineNative(vm, &klass->methods, "clock", clockNative);
    defineNative(vm, &klass->methods, "collect", collectNative);
    defineNative(vm, &klass->methods, "sleep", sleepNative);
    defineNative(vm, &klass->methods, "exit", exitNative);

    tableSet(vm, &vm->globals, name, OBJ_VAL(klass));
    pop(vm);
    pop(vm);
}
