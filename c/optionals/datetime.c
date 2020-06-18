#include "datetime.h"

static Value nowNative(VM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "now() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    time_t t = time(NULL);
    struct tm tictoc;
    char time[26];

    localtime_r(&t, &tictoc);
    asctime_r(&tictoc, time);

    // -1 to remove newline
    return OBJ_VAL(copyString(vm, time, strlen(time) - 1));
}

static Value nowUTCNative(VM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "nowUTC() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    time_t t = time(NULL);
    struct tm tictoc;
    char time[26];

    gmtime_r(&t, &tictoc);
    asctime_r(&tictoc, time);

    // -1 to remove newline
    return OBJ_VAL(copyString(vm, time, strlen(time) - 1));
}

static Value strftimeNative(VM *vm, int argCount, Value *args) {
    if (argCount != 1 && argCount != 2) {
        runtimeError(vm, "strftime() takes 1 or 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "strftime() argument must be a string");
        return EMPTY_VAL;
    }

    time_t t;

    if (argCount == 2) {
        if (!IS_NUMBER(args[1])) {
            runtimeError(vm, "strftime() optional argument must be a number");
            return EMPTY_VAL;
        }

        t = AS_NUMBER(args[1]);
    } else {
        time(&t);
    }

    struct tm tictoc;
    int len = 100;
    char buffer[100], *point = buffer;

    gmtime_r(&t, &tictoc);

    /**
     * strtime returns 0 when it fails to write - this would be due to the buffer
     * not being large enough. In that instance we double the buffer length until
     * there is a big enough buffer.
     */
    while (strftime(point, sizeof(char) * len, AS_CSTRING(args[0]), &tictoc) == 0) {
        len *= 2;
        if (buffer != point)
            free(point);

        point = malloc(len);
    }

    if (buffer != point)
        free(point);

    return OBJ_VAL(copyString(vm, point, strlen(point)));
}

static Value strptimeNative(VM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "strptime() takes 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0]) || !IS_STRING(args[1])) {
        runtimeError(vm, "strptime() arguments must be strings");
        return EMPTY_VAL;
    }

    struct tm tictoc = {0};
    tictoc.tm_mday = 1;
    tictoc.tm_isdst = -1;

    char *end = strptime(AS_CSTRING(args[1]), AS_CSTRING(args[0]), &tictoc);

    if (end == NULL) {
        return NIL_VAL;
    }

    return NUMBER_VAL((double) mktime(&tictoc));
}

void createDatetimeClass(VM *vm) {
    ObjString *name = copyString(vm, "Datetime", 8);
    push(vm, OBJ_VAL(name));
    ObjClassNative *klass = newClassNative(vm, name);
    push(vm, OBJ_VAL(klass));

    /**
     * Define Datetime methods
     */
    defineNative(vm, &klass->methods, "now", nowNative);
    defineNative(vm, &klass->methods, "nowUTC", nowUTCNative);
    defineNative(vm, &klass->methods, "strftime", strftimeNative);
    defineNative(vm, &klass->methods, "strptime", strptimeNative);

    tableSet(vm, &vm->globals, name, OBJ_VAL(klass));
    pop(vm);
    pop(vm);
}