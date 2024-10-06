#include <stdlib.h>

#include "datetime.h"


#ifdef _WIN32
#define localtime_r(TIMER, BUF) localtime_s(BUF, TIMER)
// Assumes length of BUF is 26
#define asctime_r(TIME_PTR, BUF) (asctime_s(BUF, 26, TIME_PTR), BUF)
#define gmtime_r(TIMER, BUF) gmtime_s(BUF, TIMER)
#endif

#define ISO8601Format "%m/%d/%Y %H:%M:%S %Z"
#define RFC3339Format "%Y-%m-%dT%T%Z"
#define RFC2822Format "%a, %d %b %Y %T %z"

#define NANOSECOND  (long long)1
#define MICROSECOND (long long)(1000 * NANOSECOND)
#define MILLISECOND (long long)(1000 * MICROSECOND)
#define SECOND      (long long)(1000 * MILLISECOND)
#define MINUTE      (long long)(60 * SECOND)
#define HOUR        (long long)(60 * MINUTE)

typedef struct {
    long time;
    bool isLocal; 
} Datetime;

#define AS_DATETIME(v) ((Datetime*)AS_ABSTRACT(v)->data)

static Value datetimeUTC(DictuVM *vm, int argCount, Value *args);
static Value datetimeBefore(DictuVM *vm, int argCount, Value *args);
static Value datetimeAfter(DictuVM *vm, int argCount, Value *args);
static Value datetimeClock(DictuVM *vm, int argCount, Value *args);
static Value datetimeDaysSinceJan(DictuVM *vm, int argCount, Value *args);
static Value datetimeMonthsSinceJan(DictuVM *vm, int argCount, Value *args);
static Value datetimeIsDST(DictuVM *vm, int argCount, Value *args);
static Value datetimeDaysSinceSunday(DictuVM *vm, int argCount, Value *args);
static Value datetimeYearsSince1900(DictuVM *vm, int argCount, Value *args);
static Value datetimeSecondsAfterMinute(DictuVM *vm, int argCount, Value *args);
static Value datetimeMinutesAfterHour(DictuVM *vm, int argCount, Value *args);
static Value datetimeDayOfMonth(DictuVM *vm, int argCount, Value *args);
static Value datetimeEqual(DictuVM *vm, int argCount, Value *args);
static Value datetimeTimezone(DictuVM *vm, int argCount, Value *args);

void freeDatetime(DictuVM *vm, ObjAbstract *abstract) {
    FREE(vm, Datetime, abstract->data);
}

Datetime *createDatetime(DictuVM *vm) {
    Datetime *datetime = ALLOCATE(vm, Datetime, 1);
    return datetime;
}

char *datetimeTypeToString(ObjAbstract *abstract) {
    UNUSED(abstract);

    char *datetimeString = malloc(sizeof(char) * 11);
    snprintf(datetimeString, 11, "<Datetime>");
    return datetimeString;
}

#define DEFAULT_DATETIME_FORMAT "%a %b %d %H:%M:%S %Y"

static Value datetimeFormat(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "format() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    char *fmt;
    int len;

    const int defaultLength = 128;

    if (argCount == 1) {
        if (!IS_STRING(args[1])) {
            runtimeError(vm, "format() argument must be a string");
            return EMPTY_VAL;
        }

        ObjString *format = AS_STRING(args[1]);
        // this is to avoid an eternal loop while calling format() below
        if (format->length == 0) {
            return OBJ_VAL(copyString(vm, "", 0));
        }

        fmt = format->chars;
        len = (format->length > defaultLength ? format->length * 4 : defaultLength);
    } else {
        len = defaultLength;
        fmt = DEFAULT_DATETIME_FORMAT;
    }

    char *point = ALLOCATE(vm, char, len);
    if (point == NULL) {
        runtimeError(vm, "Memory error on format()!");
        return EMPTY_VAL;
    }

    Datetime *datetime = AS_DATETIME(args[0]);

    struct tm tm;
    if (datetime->isLocal) {
        localtime_r(&datetime->time, &tm);
    } else{
        gmtime_r(&datetime->time, &tm);
    }
    tm.tm_isdst = -1;

    const int maxIterations = 8;

    int iterator = 0;
    while (strftime(point, sizeof(char) * len, fmt, &tm) == 0) {
        if (++iterator > maxIterations) {
            FREE_ARRAY(vm, char, point, len);
            return OBJ_VAL(copyString(vm, "", 0));
        }

        len *= 2;

        point = GROW_ARRAY(vm, point, char, len / 2, len);
        if (point == NULL) {
            runtimeError(vm, "Memory error on format()!");
            return EMPTY_VAL;
        }
    }

    int length = strlen(point);
    if (length != len) {
        point = SHRINK_ARRAY(vm, point, char, len, length + 1);
    }

    return OBJ_VAL(takeString(vm, point, length));
}

static Value datetimeToString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "toString() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    int len = 128;
    char *point = ALLOCATE(vm, char, len);
    if (point == NULL) {
        runtimeError(vm, "Memory error on format()");
        return EMPTY_VAL;
    }

    Datetime *datetime = AS_DATETIME(args[0]);

    struct tm tm;
    if (datetime->isLocal) {
        localtime_r(&datetime->time, &tm);
    } else{
        gmtime_r(&datetime->time, &tm);
    }
    tm.tm_isdst = -1;
    const int maxIterations = 8;

    int iterator = 0;
    while (strftime(point, sizeof(char) * len, DEFAULT_DATETIME_FORMAT, &tm) == 0) {
        if (++iterator > maxIterations) {
            FREE_ARRAY(vm, char, point, len);
            return OBJ_VAL(copyString(vm, "", 0));
        }

        len *= 2;

        point = GROW_ARRAY(vm, point, char, len / 2, len);
        if (point == NULL) {
            runtimeError(vm, "Memory error on format()");
            return EMPTY_VAL;
        }
    }

    int length = strlen(point);
    if (length != len) {
        point = SHRINK_ARRAY(vm, point, char, len, length + 1);
    }

    return OBJ_VAL(takeString(vm, point, length));
}

static Value datetimeUnix(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "unix() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Datetime *datetime = AS_DATETIME(args[0]);

    return NUMBER_VAL(datetime->time);
}

ObjAbstract *newDatetimeObj(DictuVM *vm, long time, int isLocal) {
    UNUSED(isLocal);
    ObjAbstract *abstract = newAbstract(vm, freeDatetime, datetimeTypeToString);
    push(vm, OBJ_VAL(abstract));

    Datetime *datetime = createDatetime(vm);
    datetime->time = time;
    datetime->isLocal = isLocal;

    /**
     * Setup Datetime object methods
     */
    defineNative(vm, &abstract->values, "unix", datetimeUnix);
    defineNative(vm, &abstract->values, "utc", datetimeUTC);
    defineNative(vm, &abstract->values, "before", datetimeBefore);
    defineNative(vm, &abstract->values, "after", datetimeAfter);
    defineNative(vm, &abstract->values, "clock", datetimeClock);
    defineNative(vm, &abstract->values, "daysSinceJanFirst", datetimeDaysSinceJan);
    defineNative(vm, &abstract->values, "monthsSinceJan", datetimeMonthsSinceJan);
    defineNative(vm, &abstract->values, "isDST", datetimeIsDST);
    defineNative(vm, &abstract->values, "daysSinceSunday", datetimeDaysSinceSunday);
    defineNative(vm, &abstract->values, "yearsSince1900", datetimeYearsSince1900);
    defineNative(vm, &abstract->values, "secondsAfterMinute", datetimeSecondsAfterMinute);
    defineNative(vm, &abstract->values, "minutesAfterHour", datetimeMinutesAfterHour);
    defineNative(vm, &abstract->values, "dayOfMonth", datetimeDayOfMonth);
    defineNative(vm, &abstract->values, "equal", datetimeEqual);
    defineNative(vm, &abstract->values, "timezone", datetimeTimezone);
    defineNative(vm, &abstract->values, "toString", datetimeToString);
    defineNative(vm, &abstract->values, "format", datetimeFormat);

    abstract->data = datetime;
    pop(vm);

    return abstract;
}

static Value datetimeUTC(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "utc() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Datetime *datetime = AS_DATETIME(args[0]);

    struct tm tm;
    gmtime_r(&datetime->time, &tm);

    return OBJ_VAL(newDatetimeObj(vm, (long)datetime->time, false));
}

static Value datetimeBefore(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "before() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    Datetime *d1 = AS_DATETIME(args[0]);
    Datetime *d2 = AS_DATETIME(args[1]);

    return BOOL_VAL(d2->time < d1->time);
}

static Value datetimeAfter(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "after() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    Datetime *d1 = AS_DATETIME(args[0]);
    Datetime *d2 = AS_DATETIME(args[1]);

    return BOOL_VAL(d2->time > d1->time);
}

static Value datetimeClock(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "clock() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Datetime *d1 = AS_DATETIME(args[0]);

    ObjDict *dict = newDict(vm);
    push(vm, OBJ_VAL(dict));

    struct tm *t = localtime((time_t*)&d1->time);
    Value hour = OBJ_VAL(copyString(vm, "hour", 4));
    push(vm, hour);
    dictSet(vm, dict, hour, NUMBER_VAL(t->tm_hour));
    pop(vm);

    Value min = OBJ_VAL(copyString(vm, "min", 3));
    push(vm, min);
    dictSet(vm, dict, min, NUMBER_VAL(t->tm_min));
    pop(vm);

    Value sec = OBJ_VAL(copyString(vm, "sec", 3));
    push(vm, sec);
    dictSet(vm, dict, sec, NUMBER_VAL(t->tm_sec));
    pop(vm);
    
    pop(vm);

    return OBJ_VAL(dict);
}

static Value datetimeDaysSinceJan(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "daysSinceJanOne() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Datetime *d1 = AS_DATETIME(args[0]);
    struct tm *t = localtime((time_t*)&d1->time);

    return NUMBER_VAL(t->tm_yday);
}

static Value datetimeMonthsSinceJan(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "monthsSinceJan() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Datetime *d1 = AS_DATETIME(args[0]);
    struct tm *t = localtime((time_t*)&d1->time);

    return NUMBER_VAL(t->tm_mon);
}

static Value datetimeIsDST(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "isDST() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Datetime *d1 = AS_DATETIME(args[0]);
    struct tm *t = localtime((time_t*)&d1->time);

    return BOOL_VAL(t->tm_isdst == true);
}

static Value datetimeDaysSinceSunday(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "daysSinceSunday() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Datetime *d1 = AS_DATETIME(args[0]);
    struct tm *t = localtime((time_t*)&d1->time);

    return NUMBER_VAL(t->tm_wday);
}

static Value datetimeYearsSince1900(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "yearsSince1900() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Datetime *d1 = AS_DATETIME(args[0]);
    struct tm *t = localtime((time_t*)&d1->time);

    return NUMBER_VAL(t->tm_year);
}

static Value datetimeSecondsAfterMinute(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "secondsAfterMinute() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Datetime *d1 = AS_DATETIME(args[0]);
    struct tm *t = localtime((time_t*)&d1->time);

    return NUMBER_VAL(t->tm_sec);
}

static Value datetimeMinutesAfterHour(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "minutesAfterHour() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Datetime *d1 = AS_DATETIME(args[0]);
    struct tm *t = localtime((time_t*)&d1->time);

    return NUMBER_VAL(t->tm_min);
}

static Value datetimeDayOfMonth(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "dayOfMonth() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Datetime *d1 = AS_DATETIME(args[0]);
    struct tm *t = localtime((time_t*)&d1->time);

    return NUMBER_VAL(t->tm_mday);
}

static Value datetimeEqual(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "equal() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    Datetime *d1 = AS_DATETIME(args[0]);
    Datetime *d2 = AS_DATETIME(args[1]);

    return BOOL_VAL((d1->isLocal == d2->isLocal) && (d1->time == d2->time));
}

static Value datetimeTimezone(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "timezone() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Datetime *d1 = AS_DATETIME(args[0]);
    struct tm *t = localtime((time_t*)&d1->time);

    return OBJ_VAL(copyString(vm, t->tm_zone, strlen(t->tm_zone)));
}

static Value newDatetimeNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount > 1 || argCount < 0) {
        runtimeError(vm, "new() takes 0 or 1 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (argCount == 0) {
        time_t t = time(NULL);
        struct tm tm;
        localtime_r(&t, &tm);

        return OBJ_VAL(newDatetimeObj(vm, (long)t, true));
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError(vm, "argument must be a number");
        return EMPTY_VAL;
    }

    time_t num = (time_t)((long)AS_NUMBER(args[0]));

    return OBJ_VAL(newDatetimeObj(vm, (long)num, false));
}

static Value isLeapYearNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "isLeapYear() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError(vm, "argument must be a number");
        return EMPTY_VAL;
    }

    long year = (long)AS_NUMBER(args[0]);

    return BOOL_VAL((year & 3) == 0 && ((year % 25) != 0 || (year & 15) == 0));
}

#ifndef _WIN32
static Value parseNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "parse() takes 2 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "first argument must be a number");
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "second argument must be a number");
        return EMPTY_VAL;
    }

    char *datetimeStr = AS_CSTRING(args[0]);
    char *format = AS_CSTRING(args[1]);

    struct tm ts;

    char *res = strptime(datetimeStr, format, &ts);
    if (res == NULL) {
        runtimeError(vm, "failed to parse datetime string");
        return EMPTY_VAL;
    }

    Datetime *datetime = createDatetime(vm);
    

    return OBJ_VAL(newDatetimeObj(vm, (long)datetime->time, false));
}
#endif

Value createDatetimeModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Datetime", 8);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Datetime methods
     */
    defineNative(vm, &module->values, "new", newDatetimeNative);
    defineNative(vm, &module->values, "isLeapYear", isLeapYearNative);
#ifndef _WIN32
    defineNative(vm, &module->values, "parse", parseNative);
#endif

    /**
     * Define Datetime properties
     */
    defineNativeProperty(vm, &module->values, "NANOSECOND", NUMBER_VAL(NANOSECOND));
    defineNativeProperty(vm, &module->values, "MICROSECOND", NUMBER_VAL(MICROSECOND));
    defineNativeProperty(vm, &module->values, "MILLISECOND", NUMBER_VAL(MILLISECOND));
    defineNativeProperty(vm, &module->values, "SECOND", NUMBER_VAL(SECOND));
    defineNativeProperty(vm, &module->values, "MINUTE", NUMBER_VAL(MINUTE));
    defineNativeProperty(vm, &module->values, "HOUR", NUMBER_VAL(HOUR));

    defineNativeProperty(vm, &module->values, "SECONDS_IN_MINUTE", NUMBER_VAL(60));
    defineNativeProperty(vm, &module->values, "SECONDS_IN_HOUR", NUMBER_VAL(3600));
    defineNativeProperty(vm, &module->values, "SECONDS_IN_DAY", NUMBER_VAL(86400));
    defineNativeProperty(vm, &module->values, "SECONDS_IN_WEEK", NUMBER_VAL(604800));

    defineNativeProperty(vm, &module->values, "DAYS_PER_400_YEARS", NUMBER_VAL(365*400 + 97));
    defineNativeProperty(vm, &module->values, "DAYS_PER_100_YEARS", NUMBER_VAL(365*100 + 24));
    defineNativeProperty(vm, &module->values, "DAYS_PER_4_YEARS", NUMBER_VAL(365*4 + 1));

    defineNativeProperty(vm, &module->values, "RFC3339", OBJ_VAL(copyString(vm, RFC3339Format, strlen(RFC3339Format))));
    defineNativeProperty(vm, &module->values, "RFC2822", OBJ_VAL(copyString(vm, RFC2822Format, strlen(RFC2822Format))));
    defineNativeProperty(vm, &module->values, "ISO8601", OBJ_VAL(copyString(vm, ISO8601Format, strlen(ISO8601Format))));

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
