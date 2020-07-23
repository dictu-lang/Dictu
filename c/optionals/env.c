#include "env.h"

static Value get(VM *vm, int argCount, Value *args) {
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

    /* getenv() doesn't set errno, so we provide an own error */
    errno = EINVAL; /* EINVAL seems appropriate */

    SET_ERRNO(GET_SELF_CLASS);

    return NIL_VAL;
}

static Value set(VM *vm, int argCount, Value *args) {
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
        SET_ERRNO(GET_SELF_CLASS);
    }

    return NUMBER_VAL(retval == 0 ? OK : NOTOK);
}

ObjModule *createEnvClass(VM *vm) {
    ObjString *name = copyString(vm, "Env", 3);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Env methods
     */
    defineNative(vm, &module->values, "strerror", strerrorNative);
    defineNative(vm, &module->values, "get", get);
    defineNative(vm, &module->values, "set", set);
    pop(vm);
    pop(vm);

    return module;
}
