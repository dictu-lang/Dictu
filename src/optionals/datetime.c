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

#define AS_DATETIME(v) ((Datetime*)AS_ABSTRACT(v)->data)
#define AS_DURATION(v) ((Duration*)AS_ABSTRACT(v)->data)


static Value datetimeAddDuration(DictuVM *vm, int argCount, Value *args);
static Value datetimeSubDuration(DictuVM *vm, int argCount, Value *args);
static Value datetimeDelta(DictuVM *vm, int argCount, Value *args);


void freeDatetime(DictuVM *vm, ObjAbstract *abstract) {
    FREE(vm, Datetime, abstract->data);
}


Datetime* createDatetime(DictuVM *vm) {
    Datetime *datetime = ALLOCATE(vm, Datetime, 1);
    return datetime;
}

char *datetimeTypeToString(ObjAbstract *abstract) {
    UNUSED(abstract);

    char *datetimeString = malloc(sizeof(char) * 11);
    snprintf(datetimeString, 11, "<Datetime>");
    return datetimeString;
}

static Value datetimeStrftime(DictuVM *vm, int argCount, Value *args){


    if (argCount > 1) {
        runtimeError(vm, "strftime() takes 0 or 1 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    char *fmt;
    int len;

    int default_length = 128;

    if(argCount == 1) {
        if (!IS_STRING(args[1])) {
            runtimeError(vm, "strftime() argument must be a string");
            return EMPTY_VAL;
        }
        ObjString *format = AS_STRING(args[1]);
        /** this is to avoid an eternal loop while calling strftime() below */
        if (0 == format->length)
        return OBJ_VAL(copyString(vm, "", 0));

        fmt = format->chars;
        len = (format->length > default_length ? format->length * 4 : default_length);
    }

    else {
        len = default_length;
        fmt = "%a %b %d %H:%M:%S %Y";
    }

    char *point = ALLOCATE(vm, char, len);
    if (point == NULL) {
        runtimeError(vm, "Memory error on strftime()!");
        return EMPTY_VAL;
    }

    Datetime *datetime = AS_DATETIME(args[0]);
    struct tm tictoc;
    if(datetime->is_local){
        localtime_r(&datetime->time, &tictoc);
    }
    else{
        gmtime_r(&datetime->time, &tictoc);
    }
    tictoc.tm_isdst = -1;
    
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
static Value datetimeGetTime(DictuVM *vm, int argCount, Value *args){

    if (argCount != 0) {
        runtimeError(vm, "getTime() take 0 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    Datetime *datetime = AS_DATETIME(args[0]);
    return NUMBER_VAL(datetime->time);
}
#endif


ObjAbstract* newDatetimeObj( DictuVM *vm, long time, int is_local) {
    ObjAbstract *abstract = newAbstract(vm, freeDatetime, datetimeTypeToString);
    push(vm, OBJ_VAL(abstract));

    Datetime *datetime = createDatetime(vm);
    datetime->time = time;
    datetime->is_local = is_local;
    /**
     * Setup Datetime object methods
     */

    defineNative(vm, &abstract->values, "strftime", datetimeStrftime);
    #ifdef HAS_STRPTIME
    defineNative(vm, &abstract->values, "getTime", datetimeGetTime);
    #endif

    defineNative(vm, &abstract->values, "add", datetimeAddDuration);
    defineNative(vm, &abstract->values, "sub", datetimeSubDuration);
    defineNative(vm, &abstract->values, "delta", datetimeDelta);

    abstract->data = datetime;
    pop(vm);

    return abstract;
}

static Value nowNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "now() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    time_t t = time(NULL);
    struct tm tictoc;
    localtime_r(&t, &tictoc);

    return OBJ_VAL(newDatetimeObj(vm, (long)t, true));
}

static Value nowUTCNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "nowUTC() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    time_t t = time(NULL);
    struct tm tictoc;
    gmtime_r(&t, &tictoc);

    return OBJ_VAL(newDatetimeObj(vm, (long)t, false));
}


static Value newUTCDatetimeNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "nowUTC() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    time_t t = time(NULL);
    struct tm tictoc;
    gmtime_r(&t, &tictoc);
    return OBJ_VAL(newDatetimeObj(vm, (long)t, false));
}


static Value newDatetimeNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if(argCount == 1) {
        if (!IS_NUMBER(args[0])) {
            runtimeError(vm, "new() argument must be a number");
            return EMPTY_VAL;
        }

        time_t num = (time_t)((long) AS_NUMBER(args[0]));
        return OBJ_VAL(newDatetimeObj(vm, (long)num, false));
    }

    if (argCount == 0) {
        time_t t = time(NULL);
        struct tm tictoc;
        localtime_r(&t, &tictoc);

        return OBJ_VAL(newDatetimeObj(vm, (long)t, true));
    }

    runtimeError(vm, "new() takes 0 or 1 arguments (%d given)", argCount);

    return EMPTY_VAL;
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

    if (strptime(AS_CSTRING(args[1]), AS_CSTRING(args[0]), &tictoc) != NULL) {
        const time_t timestamp = mktime(&tictoc);
        return OBJ_VAL(newDatetimeObj(vm, (long)timestamp+tictoc.tm_gmtoff, false));
    }

    return NIL_VAL;
}
#endif


// Duration object methods

void freeDuration(DictuVM *vm, ObjAbstract *abstract) {
    FREE(vm, Duration, abstract->data);
}

Duration* createDuration(DictuVM *vm) {
    Duration *duration = ALLOCATE(vm, Duration, 1);
    return duration;
}


char *durationTypeToString(ObjAbstract *abstract) {
    UNUSED(abstract);

    char *durationString = malloc(sizeof(char) * 11);
    snprintf(durationString, 11, "<Duration>");
    return durationString;
}

static Value datetimeAddDuration(DictuVM *vm, int argCount, Value *args){

    if (argCount != 1) {
        runtimeError(vm, "add() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }
    Datetime *datetime = AS_DATETIME(args[0]);
    Duration *duration = AS_DURATION(args[1]);

    struct tm* timeinfo = localtime(&datetime->time);

    timeinfo->tm_sec += duration->seconds;

    long newTime = mktime(timeinfo);

    return OBJ_VAL(newDatetimeObj(vm, newTime, datetime->is_local));
}

static Value datetimeSubDuration(DictuVM *vm, int argCount, Value *args){

    if (argCount != 1) {
        runtimeError(vm, "sub() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }
    Datetime *datetime = AS_DATETIME(args[0]);
    Duration *duration = AS_DURATION(args[1]);

    struct tm* timeinfo = localtime(&datetime->time);

    timeinfo->tm_sec -= duration->seconds;

    long newTime = mktime(timeinfo);

    return OBJ_VAL(newDatetimeObj(vm, newTime, datetime->is_local));
}

ObjAbstract* newDurationObj( DictuVM *vm, unsigned long seconds) {
    ObjAbstract *abstract = newAbstract(vm, freeDuration, durationTypeToString);
    push(vm, OBJ_VAL(abstract));

    Duration *duration = createDuration(vm);
    duration->seconds = seconds;

    abstract->data = duration;
    pop(vm);

    return abstract;
}

static Value datetimeDelta(DictuVM *vm, int argCount, Value *args) {
    
    if (argCount != 1) {
        runtimeError(vm, "delta() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    Datetime *datetimeOne = AS_DATETIME(args[0]);
    Datetime *datetimeTwo = AS_DATETIME(args[1]);

    return OBJ_VAL(newDurationObj(vm, labs(datetimeOne->time - datetimeTwo->time)));
}


#define HANDLE_DURATION_PARAM(key, field) \
    if (strstr(key, #field)) { \
        if (!IS_NUMBER(entry->value)) { \
            runtimeError(vm, "Duration parameter key \"" #field "\" value must be a number"); \
            return EMPTY_VAL; \
        } \
        field = (unsigned int)AS_NUMBER(entry->value); \
    }

static Value newDurationNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 1) {
        runtimeError(vm, "duration() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_DICT(args[0])) {
        runtimeError(vm, "duration() argument must be a dictionary");
        return EMPTY_VAL;
    }

    ObjDict *opts = AS_DICT(args[0]);

    unsigned int days = 0;
    unsigned int seconds = 0;
    unsigned int minutes = 0;
    unsigned int hours = 0;
    unsigned int weeks = 0;

    if (opts->count != 0) {
        for (int i = 0; i <= opts->capacityMask; i++) {
            DictItem *entry = &opts->entries[i];
            if (IS_EMPTY(entry->key)) {
                continue;
            }

            char *key;

            if (IS_STRING(entry->key)) {
                ObjString *s = AS_STRING(entry->key);
                key = s->chars;
            } else {
                runtimeError(vm, "Duration parameter key must be a string");
                return EMPTY_VAL;
            }


            HANDLE_DURATION_PARAM(key, days)
            else HANDLE_DURATION_PARAM(key, seconds)
            else HANDLE_DURATION_PARAM(key, minutes)
            else HANDLE_DURATION_PARAM(key, hours)
            else HANDLE_DURATION_PARAM(key, weeks)
            else {
                runtimeError(vm, "unknown parameter key '%s' found in duration dictionary", key);
                return EMPTY_VAL;
            }
        }
    }

    unsigned long final_seconds  = seconds + (60*minutes) + (3600*hours) + (24 * 3600 * days) + (24 * 3600 * 7 * weeks);

    return OBJ_VAL(newDurationObj(vm, final_seconds));
}


Value createDatetimeModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Datetime", 8);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Datetime methods
     */
    
    defineNative(vm, &module->values, "new", newDatetimeNative);
    defineNative(vm, &module->values, "newUTC", newUTCDatetimeNative);
    defineNative(vm, &module->values, "now", nowNative);
    defineNative(vm, &module->values, "nowUTC", nowUTCNative);

    #ifdef HAS_STRPTIME
    defineNative(vm, &module->values, "strptime", strptimeNative);
    #endif

    defineNative(vm, &module->values, "duration", newDurationNative);

    /**
     * Define Datetime properties
     */
    defineNativeProperty(vm, &module->values, "SECONDS_IN_MINUTE", NUMBER_VAL(60));
    defineNativeProperty(vm, &module->values, "SECONDS_IN_HOUR", NUMBER_VAL(3600));
    defineNativeProperty(vm, &module->values, "SECONDS_IN_DAY", NUMBER_VAL(86400));
    defineNativeProperty(vm, &module->values, "SECONDS_IN_WEEK", NUMBER_VAL(604800));

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
