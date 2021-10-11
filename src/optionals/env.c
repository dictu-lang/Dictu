#include "env.h"

#ifdef _WIN32
#define unsetenv(NAME) _putenv_s(NAME, "")
int setenv(const char *name, const char *value, int overwrite) {
    if (!overwrite && getenv(name) == NULL) {
        return 0;
    }

    if (_putenv_s(name, value)) {
        return -1;
    } else {
        return 0;
    }
}
#endif

static Value get(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "get() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "get() argument must be a string.");
        return EMPTY_VAL;
    }

    char *value = getenv(AS_CSTRING(args[0]));

    if (value != NULL) {
        return OBJ_VAL(copyString(vm, value, strlen(value)));
    }

    return NIL_VAL;
}

static Value getOrDefault(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "getOrDefault() takes 2 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])||!IS_STRING(args[1])) {
        runtimeError(vm, "getOrDefault() arguments must be a string.");
        return EMPTY_VAL;
    }

    char *value = getenv(AS_CSTRING(args[0]));
    char *defaultValue = AS_CSTRING(args[1]);

    if (defaultValue == NULL) {
        runtimeError(vm, "getOrDefault() second argument must not be null");
        return EMPTY_VAL;
    }

    if (value != NULL) {
        return OBJ_VAL(copyString(vm, value, strlen(value)));
    }

    return OBJ_VAL(copyString(vm, defaultValue, strlen(defaultValue)));
}

static Value set(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "set() takes 2 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0]) || (!IS_STRING(args[1]) && !IS_NIL(args[1]))) {
        runtimeError(vm, "set() arguments must be a string or nil.");
        return EMPTY_VAL;
    }

    char *key = AS_CSTRING(args[0]);

    int retval;
    if (IS_NIL(args[1])) {
        retval = unsetenv(key);
    } else {
        retval = setenv(key, AS_CSTRING(args[1]), 1);
    }

    /* both set errno, though probably they can not fail */
    if (retval == NOTOK) {
        return newResultError(vm, "Failed to set environment variable");
    }

    return newResultSuccess(vm, NIL_VAL);
}

ObjModule *createEnvModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Env", 3);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Env methods
     */
    defineNative(vm, &module->values, "get", get);
    defineNative(vm, &module->values, "getOrDefault", getOrDefault);
    defineNative(vm, &module->values, "set", set);

    pop(vm);
    pop(vm);

    return module;
}
