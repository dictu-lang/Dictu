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

#define ISO8601Format "%m/%d/%Y %H:%M:%S %Z"
#define RFC3339Format "%Y-%m-%dT%T%Z"
#define RFC2822Format "%a, %d %b %Y %T %z"

// #define MIN_DURATION -1 << 63
// #define MAX_DURATION 1<<63 - 1

// #define DAY 1
// #define DAYS_IN_WEEK 7
// #define DAYS_IN_MONTH 30

// #define SECOND 1
// #define SECONDS_IN_MINUTE 60
// #define SECONDS_IN_HOUR SECONDS_IN_MINUTE * 60
// #define SECONDS_IN_DAY SECONDS_IN_HOUR * 24
// #define SECONDS_IN_WEEK SECONDS_IN_DAY * 7
// #define SECONDS_IN_MONTH 30*24*60*60

typedef struct {
    long time;
    bool isLocal; 
} Datetime;

enum Durations {
    NANOSECOND,
    MICROSECOND,
    MILLISECOND,
    SECOND,
    MINUTE,
    HOUR,
    DAY
};

typedef struct {
    enum Durations type;
    long long nanoseconds;
} Duration;

#define AS_DATETIME(v) ((Datetime*)AS_ABSTRACT(v)->data)
#define AS_DURATION(v) ((Duration*)AS_ABSTRACT(v)->data)

static Value datetimeAddDuration(DictuVM *vm, int argCount, Value *args);
static Value datetimeSubDuration(DictuVM *vm, int argCount, Value *args);
static Value datetimeDiff(DictuVM *vm, int argCount, Value *args);
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
static ObjAbstract* newDurationObj(DictuVM *vm, long long nanoseconds);

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

    int defaultLength = 128;

    if (argCount == 1) {
        if (!IS_STRING(args[1])) {
            runtimeError(vm, "format() argument must be a string");
            return EMPTY_VAL;
        }

        ObjString *format = AS_STRING(args[1]);
        /** this is to avoid an eternal loop while calling format() below */
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

    struct tm tictoc;
    if(datetime->isLocal){
        localtime_r(&datetime->time, &tictoc);
    } else{
        gmtime_r(&datetime->time, &tictoc);
    }
    tictoc.tm_isdst = -1;

    const int maxIterations = 8;

    int iterator = 0;
    while (strftime(point, sizeof(char) * len, fmt, &tictoc) == 0) {
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
    if (argCount > 0) {
        runtimeError(vm, "toString() takes 1 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    int len = 128;
    //char *fmt = DEFAULT_DATETIME_FORMAT;

    char *point = ALLOCATE(vm, char, len);
    if (point == NULL) {
        runtimeError(vm, "Memory error on format()");
        return EMPTY_VAL;
    }

    Datetime *datetime = AS_DATETIME(args[0]);

    struct tm tictoc;
    if(datetime->isLocal){
        localtime_r(&datetime->time, &tictoc);
    } else{
        gmtime_r(&datetime->time, &tictoc);
    }
    tictoc.tm_isdst = -1;
    const int maxIterations = 8;

    int iterator = 0;
    while (strftime(point, sizeof(char) * len, DEFAULT_DATETIME_FORMAT, &tictoc) == 0) {
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

#ifdef HAS_STRPTIME
static Value datetimeUnix(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "unix() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Datetime *datetime = AS_DATETIME(args[0]);

    return NUMBER_VAL(datetime->time);
}
#endif

ObjAbstract *newDatetimeObj(DictuVM *vm, long time, int isLocal) {
    ObjAbstract *abstract = newAbstract(vm, freeDatetime, datetimeTypeToString);
    push(vm, OBJ_VAL(abstract));

    Datetime *datetime = createDatetime(vm);
    datetime->time = time;
    datetime->isLocal = isLocal;

    /**
     * Setup Datetime object methods
     */
    #ifdef HAS_STRPTIME
    defineNative(vm, &abstract->values, "unix", datetimeUnix);
    #endif

    defineNative(vm, &abstract->values, "utc", datetimeUTC);
    defineNative(vm, &abstract->values, "add", datetimeAddDuration);
    defineNative(vm, &abstract->values, "sub", datetimeSubDuration);
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
    defineNative(vm, &abstract->values, "diff", datetimeDiff);
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

    if (datetime->isLocal) {
        return OBJ_VAL(newDatetimeObj(vm, (long)datetime->time, true));
    }

    struct tm tictoc;
    gmtime_r(&datetime->time, &tictoc);

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

static Value datetimeAddDuration(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "add() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    Datetime *datetime = AS_DATETIME(args[0]);

    struct tm *timeinfo = localtime(&datetime->time);
    timeinfo->tm_sec += (long)AS_NUMBER(args[1]);
    long newTime = mktime(timeinfo);

    return OBJ_VAL(newDatetimeObj(vm, newTime, datetime->isLocal));
}

static Value datetimeSubDuration(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "sub() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }
    Datetime *datetime = AS_DATETIME(args[0]);

    struct tm *timeinfo = localtime(&datetime->time);
    timeinfo->tm_sec -= (long)AS_NUMBER(args[1]);
    long newTime = mktime(timeinfo);

    return OBJ_VAL(newDatetimeObj(vm, newTime, datetime->isLocal));
}

static Value datetimeDiff(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "delta() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    Datetime *dt1 = AS_DATETIME(args[0]);
    Datetime *dt2 = AS_DATETIME(args[1]);

    return OBJ_VAL(newDurationObj(vm, labs(dt1->time - dt2->time)));
}

static Value newDatetimeNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount > 1 || argCount < 0) {
        runtimeError(vm, "new() takes 0 or 1 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (argCount == 0) {
        time_t t = time(NULL);
        struct tm tictoc;
        localtime_r(&t, &tictoc);

        return OBJ_VAL(newDatetimeObj(vm, (long)t, true));
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError(vm, "new() argument must be a number");
        return EMPTY_VAL;
    }

    time_t num = (time_t)((long) AS_NUMBER(args[0]));

    return OBJ_VAL(newDatetimeObj(vm, (long)num, false));

    

    runtimeError(vm, "new() takes 0 or 1 arguments (%d given)", argCount);

    return EMPTY_VAL;
}

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

static const long long nanosecond = 1;
static const long long microsecond = 1000 * nanosecond;
static const long long millisecond = 1000 * microsecond;
static const long long second = 1000 * millisecond;
static const long long minute = 60 * second;
static const long long hour = 60 * minute;

static Value durationToNanoseconds(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "nanoseconds() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Duration *dur = AS_DURATION(args[0]);

    return NUMBER_VAL(dur->nanoseconds);
}

static Value durationToMilliseconds(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "milliseconds() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Duration *dur = AS_DURATION(args[0]);

    return NUMBER_VAL(dur->nanoseconds / 1e6);
}

static Value durationToMicroseconds(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "microseconds() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Duration *dur = AS_DURATION(args[0]);

    return NUMBER_VAL(dur->nanoseconds / 1000);
}

static Value durationToSeconds(DictuVM *vm, int argCount, Value *args){
    if (argCount != 0) {
        runtimeError(vm, "seconds() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Duration *dur = AS_DURATION(args[0]);

    return NUMBER_VAL(dur->nanoseconds / 1e9);
}

ObjAbstract* newDurationObj(DictuVM *vm, long long nanoseconds) {
    ObjAbstract *abstract = newAbstract(vm, freeDuration, durationTypeToString);
    push(vm, OBJ_VAL(abstract));

    Duration *duration = createDuration(vm);
    duration->nanoseconds = nanoseconds;

    /**
     * Setup Datetime object methods
     */
    defineNative(vm, &abstract->values, "nanoseconds", durationToNanoseconds);
    defineNative(vm, &abstract->values, "milliseconds", durationToMilliseconds);
    defineNative(vm, &abstract->values, "microseconds", durationToMicroseconds);
    defineNative(vm, &abstract->values, "seconds", durationToSeconds);

    abstract->data = duration;
    pop(vm);

    return abstract;
}

static Value newDurationNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount > 1) {
        runtimeError(vm, "duration() takes 0 or 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (argCount == 1) {
        if (!IS_NUMBER(args[0])) {
            runtimeError(vm, "duration() argument must be a number");
            return EMPTY_VAL;
        }

        long long val = (long long)AS_NUMBER(args[0]);
        return OBJ_VAL(newDurationObj(vm, val));
    }
    
    return OBJ_VAL(newDurationObj(vm, (long long)0));
}

static Value parseDurationNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 1) {
        runtimeError(vm, "parseDuration() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "parseDuration() argument must be a string");
        return EMPTY_VAL;
    }

    char *dur = AS_CSTRING(args[0]);
    printf("XXX - %s\n", dur);

    if (strlen(dur) < 2) {
        runtimeError(vm, "parseDuration() argument must be a duration string");
        return EMPTY_VAL; 
    }

    if (!isdigit(dur[0])) {
        runtimeError(vm, "parseDuration() argument must start with a numeric value");
        return EMPTY_VAL; 
    }

    if (!isalpha(dur[strlen(dur)-1])) {
        runtimeError(vm, "parseDuration() argument must start with a string value");
        return EMPTY_VAL; 
    }

    char durInt[2];
    durInt[0] = dur[strlen(dur)-1];
    durInt[1] = '\0';

    static const char durationIntervals[9][2] = {"ns", "us", "ms", "s", "m", "h", "d", "w", "y"};
    int found = 0;

    for (int i = 0; i < 9; i++) {
        if (strcmp(durInt, durationIntervals[i]) == 0) {
            found++;
        }
    }

    if (found != 1) {
        runtimeError(vm, "Unknown duration interval");
        return EMPTY_VAL;
    }
    
    int digitCount = 0;
    for (unsigned long i = 0; i < strlen(dur); i++) {
        if (isdigit(dur[i])) {
            digitCount++;
        }
    }

    char *durChars = ALLOCATE(vm, char, digitCount+1);
    for (int i = 0; i < digitCount; i++) {
        durChars[i] = dur[i];
    }

    char *end;
    long long val = strtoll(durChars, &end, 10);

    FREE(vm, char, durChars);

    if (val == 0) {
        runtimeError(vm, "Failed to parse given duration string");
        return EMPTY_VAL;
    }
    printf("XXX - val: %lld\n", val);
    long long finalDurVal = 0;
    if (strcmp(durInt, "ns") == 0) {
        finalDurVal = val;
    } else if (strcmp(durInt, "us") == 0) {
        finalDurVal = val * 1000;
    } else if (strcmp(durInt, "ms") == 0) {
        finalDurVal = val * 1000000;
    } else if (strcmp(durInt, "s") == 0) {
        finalDurVal = val * 1e+9;
        printf("XXX - here\n");
    } else if (strcmp(durInt, "m") == 0) {
        finalDurVal = val * 6e+10;
    } else if (strcmp(durInt, "h") == 0) {
        finalDurVal = val * 3.6e+12;
    } else if (strcmp(durInt, "d") == 0) {
        finalDurVal = val * 8.64e+13;
    } else if (strcmp(durInt, "w") == 0) {
        finalDurVal = val * 6.048e+14;
    } else if (strcmp(durInt, "y") == 0) {
        finalDurVal = val * 3.154e+16;
    }
    
    printf("XXX - finalDurVal: %lld\n", finalDurVal);
    return OBJ_VAL(newDurationObj(vm, finalDurVal));
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
    defineNative(vm, &module->values, "newDuration", newDurationNative);
    defineNative(vm, &module->values, "parseDuration", parseDurationNative);

    /**
     * Define Datetime properties
     */
    defineNativeProperty(vm, &module->values, "NANOSECOND", NUMBER_VAL(nanosecond));
    defineNativeProperty(vm, &module->values, "MICROSECOND", NUMBER_VAL(microsecond));
    defineNativeProperty(vm, &module->values, "MILLISECOND", NUMBER_VAL(millisecond));
    defineNativeProperty(vm, &module->values, "SECOND", NUMBER_VAL(second));
    defineNativeProperty(vm, &module->values, "MINUTE", NUMBER_VAL(minute));
    defineNativeProperty(vm, &module->values, "HOUR", NUMBER_VAL(hour));

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
