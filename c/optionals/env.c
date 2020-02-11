#include "env.h"

static Value get(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("get() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError("get() argument must be a string.");
        return EMPTY_VAL;
    }

    char *value = getenv(AS_CSTRING(args[0]));

    if (value != NULL) {
        return OBJ_VAL(copyString(value, strlen(value)));
    }

    return NIL_VAL;
}

static Value set(int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError("set() takes 2 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0]) || (!IS_STRING(args[1]) && !IS_NIL(args[1]))) {
        runtimeError("set() arguments must be a string or nil.");
        return EMPTY_VAL;
    }

    char *key = AS_CSTRING(args[0]);

    if (IS_NIL(args[1])) {
        unsetenv(key);
    } else {
        setenv(key, AS_CSTRING(args[1]), 1);
    }

    return NIL_VAL;
}

void createEnvClass() {
    ObjString *name = copyString("Env", 3);
    push(OBJ_VAL(name));
    ObjClassNative *klass = newClassNative(name);
    push(OBJ_VAL(klass));

    /**
     * Define Env methods
     */
    defineNativeMethod(klass, "get", get);
    defineNativeMethod(klass, "set", set);

    tableSet(&vm.globals, name, OBJ_VAL(klass));
    pop();
    pop();
}