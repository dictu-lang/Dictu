#include "env.h"

#include "env-source.h"

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
    if (argCount != 1 && argCount != 2 ) {
        runtimeError(vm, "get() takes 1 or 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        int valLength = 0;
        char *val = valueTypeToString(vm, args[0], &valLength);
        runtimeError(vm, "get() arguments must be a string, got '%s'.", val);
        FREE_ARRAY(vm, char, val, valLength + 1);
        return EMPTY_VAL;
    }

    char *value = getenv(AS_CSTRING(args[0]));

    if (argCount == 2) {
        if (!IS_STRING(args[1])) {
            int valLength = 0;
            char *val = valueTypeToString(vm, args[1], &valLength);
            runtimeError(vm, "get() arguments must be a string, got '%s'.", val);
            FREE_ARRAY(vm, char, val, valLength + 1);
            return EMPTY_VAL;
        }

        if (value != NULL) {
            return OBJ_VAL(copyString(vm, value, strlen(value)));
        }

        return args[1];
    }

    if (value != NULL) {
        return OBJ_VAL(copyString(vm, value, strlen(value)));
    }

    return NIL_VAL;
}

static Value set(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "set() takes 2 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        int valLength = 0;
        char *val = valueTypeToString(vm, args[0], &valLength);
        runtimeError(vm, "set() arguments must be a string or nil, got '%s'.", val);
        FREE_ARRAY(vm, char, val, valLength + 1);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1]) && !IS_NIL(args[1])) {
        int valLength = 0;
        char *val = valueTypeToString(vm, args[1], &valLength);
        runtimeError(vm, "set() arguments must be a string or nil, got '%s'.", val);
        FREE_ARRAY(vm, char, val, valLength + 1);
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

#ifndef _WIN32
static Value clearAll(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "clearAll() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    extern char **environ;

    for (; *environ; ++environ) {
        char *name = strtok(*environ, "=");
        int ret = unsetenv(name);
        if (ret == -1) {
            ERROR_RESULT;
        }
    }
    
    return newResultSuccess(vm, NIL_VAL);
}
#endif

Value createEnvModule(DictuVM *vm) {
    ObjClosure *closure = compileModuleToClosure(vm, "Env", DICTU_ENV_SOURCE);

    if (closure == NULL) {
        return EMPTY_VAL;
    }

    push(vm, OBJ_VAL(closure));

    /**
     * Define Env methods
     */
    defineNative(vm, &closure->function->module->values, "get", get);
    defineNative(vm, &closure->function->module->values, "set", set);
#ifndef _WIN32
    defineNative(vm, &closure->function->module->values, "clearAll", clearAll);
#endif
    pop(vm);

    return OBJ_VAL(closure);
}
