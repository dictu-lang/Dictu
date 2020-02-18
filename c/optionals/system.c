#include "system.h"

static Value getCWDNative(int argCount, Value *args) {
    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        return OBJ_VAL(copyString(cwd, strlen(cwd)));
    }

    runtimeError("Error getting current directory");
    return EMPTY_VAL;
}

static Value timeNative(int argCount, Value *args) {
    return NUMBER_VAL((double) time(NULL));
}

static Value clockNative(int argCount, Value *args) {
    return NUMBER_VAL((double) clock() / CLOCKS_PER_SEC);
}

static Value collectNative(int argCount, Value *args) {
    collectGarbage();
    return NIL_VAL;
}

static Value sleepNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("sleep() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("sleep() argument must be a number");
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

static Value exitNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("exit() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("sleep() argument must be a number");
        return EMPTY_VAL;
    }

    exit(AS_NUMBER(args[0]));
}

void createSystemClass() {
    ObjString *name = copyString("System", 6);
    push(OBJ_VAL(name));
    ObjClassNative *klass = newClassNative(name);
    push(OBJ_VAL(klass));

    /**
     * Define System methods
     */
    defineNativeMethod(klass, "getCWD", getCWDNative);
    defineNativeMethod(klass, "time", timeNative);
    defineNativeMethod(klass, "clock", clockNative);
    defineNativeMethod(klass, "collect", collectNative);
    defineNativeMethod(klass, "sleep", sleepNative);
    defineNativeMethod(klass, "exit", exitNative);

    tableSet(&vm.globals, name, OBJ_VAL(klass));
    pop();
    pop();
}