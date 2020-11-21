#include <stdlib.h>

#include "datetime.h"

#ifdef _WIN32
#define localtime_r(TIMER, BUF) localtime_s(BUF, TIMER)
// Assumes length of BUF is 26
#define asctime_r(TIME_PTR, BUF) (asctime_s(BUF, 26, TIME_PTR), BUF)
#define gmtime_r(TIMER, BUF) gmtime_s(BUF, TIMER)
#else
#define HAS_STRPTIME
#endif

static Value nowNative(DictuVM *vm, int argCount, Value *args) {
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

static Value nowUTCNative(DictuVM *vm, int argCount, Value *args) {
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

static Value strftimeNative(DictuVM *vm, int argCount, Value *args) {
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

    ObjString *format = AS_STRING(args[0]);

    /** this is to avoid an eternal loop while calling strftime() below */
    if (0 == format->length)
        return OBJ_VAL(copyString(vm, "", 0));

    char *fmt = format->chars;

    struct tm tictoc;
    int len = (format->length > 128 ? format->length * 4 : 128);

    char *point = ALLOCATE(vm, char, len);
    if (point == NULL) {
        runtimeError(vm, "Memory error on strftime()!");
        return EMPTY_VAL;
    }

    gmtime_r(&t, &tictoc);

    /**
     * strtime returns 0 when it fails to write - this would be due to the buffer
     * not being large enough. In that instance we double the buffer length until
     * there is a big enough buffer.
     */

     /** however is not guaranteed that 0 indicates a failure (`man strftime' says so).
     * So we might want to catch up the eternal loop, by using a maximum iterator.
     */
    int max_iterations = 8;  // maximum 65536 bytes with the default 128 len,
                             // more if the given string is > 128
    int iterator = 0;
    while (strftime(point, sizeof(char) * len, fmt, &tictoc) == 0) {
        if (++iterator > max_iterations) {
            FREE_ARRAY(vm, char, point, len);
            return OBJ_VAL(copyString(vm, "", 0));
        }

        len *= 2;

        point = GROW_ARRAY(vm, point, char, len / 2, len);
        if (point == NULL) {
            runtimeError(vm, "Memory error on strftime()!");
            return EMPTY_VAL;
        }
    }

    int length = strlen(point);

    if (length != len) {
        point = SHRINK_ARRAY(vm, point, char, len, length + 1);
    }

    return OBJ_VAL(takeString(vm, point, length));
}

#ifdef HAS_STRPTIME
static Value strptimeNative(DictuVM *vm, int argCount, Value *args) {
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
#endif

ObjModule *createDatetimeModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Datetime", 8);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Datetime methods
     */
    defineNative(vm, &module->values, "strerror", strerrorNative);
    defineNative(vm, &module->values, "now", nowNative);
    defineNative(vm, &module->values, "nowUTC", nowUTCNative);
    defineNative(vm, &module->values, "strftime", strftimeNative);
    #ifdef HAS_STRPTIME
    defineNative(vm, &module->values, "strptime", strptimeNative);
    #endif

    /**
     * Define Datetime properties
     */
    defineNativeProperty(vm, &module->values, "errno", NUMBER_VAL(0));

    pop(vm);
    pop(vm);

    return module;
}
