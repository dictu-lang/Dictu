#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "natives.h"
#include "memory.h"
#include "vm.h"

static void defineNative(const char *name, NativeFn function) {
    push(OBJ_VAL(copyString(name, (int) strlen(name))));
    push(OBJ_VAL(newNative(function)));
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}


static void defineNativeVoid(const char *name, NativeFnVoid function) {
    push(OBJ_VAL(copyString(name, (int) strlen(name))));
    push(OBJ_VAL(newNativeVoid(function)));
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}

// Native functions
static Value timeNative(int argCount, Value *args) {
    return NUMBER_VAL((double) time(NULL));
}

static Value clockNative(int argCount, Value *args) {
    return NUMBER_VAL((double) clock() / CLOCKS_PER_SEC);
}

static Value numberNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("number() takes 1 argument (%d given).", argCount);
        return NIL_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError("number() only takes a string as an argument");
        return NIL_VAL;
    }

    char *numberString = AS_CSTRING(args[0]);
    double number = strtod(numberString, NULL);

    return NUMBER_VAL(number);
}

static Value strNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("str() takes 1 argument (%d given).", argCount);
        return NIL_VAL;
    }

    if (!IS_STRING(args[0])) {
        char *valueString = valueToString(args[0]);

        ObjString *string = copyString(valueString, strlen(valueString));
        free(valueString);

        return OBJ_VAL(string);
    }

    return args[0];
}

static Value typeNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("type() takes 1 argument (%d given).", argCount);
        return NIL_VAL;
    }

    if (IS_BOOL(args[0])) {
        return OBJ_VAL(copyString("bool", 4));
    } else if (IS_NIL(args[0])) {
        return OBJ_VAL(copyString("nil", 3));
    } else if (IS_NUMBER(args[0])) {
        return OBJ_VAL(copyString("number", 6));
    } else if (IS_OBJ(args[0])) {
        switch (OBJ_TYPE(args[0])) {
            case OBJ_CLASS:
                return OBJ_VAL(copyString("class", 5));
            case OBJ_BOUND_METHOD:
                return OBJ_VAL(copyString("method", 6));
            case OBJ_CLOSURE:
            case OBJ_FUNCTION:
                return OBJ_VAL(copyString("function", 8));
            case OBJ_STRING:
                return OBJ_VAL(copyString("string", 6));
            case OBJ_LIST:
                return OBJ_VAL(copyString("list", 4));
            case OBJ_DICT:
                return OBJ_VAL(copyString("dict", 4));
            case OBJ_NATIVE_VOID:
            case OBJ_NATIVE:
                return OBJ_VAL(copyString("native", 6));
            case OBJ_FILE:
                return OBJ_VAL(copyString("file", 4));
            default:
                break;
        }
    }

    return OBJ_VAL(copyString("Unknown Type", 12));
}

static Value lenNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("len() takes 1 argument (%d given).", argCount);
        return NIL_VAL;
    }

    if (IS_STRING(args[0])) {
        return NUMBER_VAL(AS_STRING(args[0])->length);
    } else if (IS_LIST(args[0])) {
        return NUMBER_VAL(AS_LIST(args[0])->values.count);
    } else if (IS_DICT(args[0])) {
        return NUMBER_VAL(AS_DICT(args[0])->count);
    }

    runtimeError("Unsupported type passed to len()", argCount);
    return NIL_VAL;
}

static Value sumNative(int argCount, Value *args) {
    double sum = 0;

    if (argCount == 0) {
        return NUMBER_VAL(0);
    } else if (argCount == 1 && IS_LIST(args[0])) {
        ObjList *list = AS_LIST(args[0]);
        argCount = list->values.count;
        args = list->values.values;
    }

    for (int i = 0; i < argCount; ++i) {
        Value value = args[i];
        if (!IS_NUMBER(value)) {
            runtimeError("A non-number value passed to sum()");
            return NIL_VAL;
        }
        sum = sum + AS_NUMBER(value);
    }

    return NUMBER_VAL(sum);
}

static Value minNative(int argCount, Value *args) {
    double current;

    if (argCount == 0) {
        return NUMBER_VAL(0);
    } else if (argCount == 1 && IS_LIST(args[0])) {
        ObjList *list = AS_LIST(args[0]);
        argCount = list->values.count;
        args = list->values.values;
    }

    double minimum = AS_NUMBER(args[0]);

    for (int i = 1; i < argCount; ++i) {
        Value value = args[i];
        if (!IS_NUMBER(value)) {
            runtimeError("A non-number value passed to min()");
            return NIL_VAL;
        }

        current = AS_NUMBER(value);

        if (minimum > current) {
            minimum = current;
        }
    }

    return NUMBER_VAL(minimum);
}

static Value maxNative(int argCount, Value *args) {
    if (argCount == 0) {
        return NUMBER_VAL(0);
    } else if (argCount == 1 && IS_LIST(args[0])) {
        ObjList *list = AS_LIST(args[0]);
        argCount = list->values.count;
        args = list->values.values;
    }

    double maximum = AS_NUMBER(args[0]);

    for (int i = 1; i < argCount; ++i) {
        Value value = args[i];
        if (!IS_NUMBER(value)) {
            runtimeError("A non-number value passed to max()");
            return NIL_VAL;
        }

        double current = AS_NUMBER(value);

        if (maximum < current) {
            maximum = current;
        }
    }

    return NUMBER_VAL(maximum);
}

static Value averageNative(int argCount, Value *args) {
    double average = 0;

    if (argCount == 0) {
        return NUMBER_VAL(0);
    } else if (argCount == 1 && IS_LIST(args[0])) {
        ObjList *list = AS_LIST(args[0]);
        argCount = list->values.count;
        args = list->values.values;
    }

    for (int i = 0; i < argCount; ++i) {
        Value value = args[i];
        if (!IS_NUMBER(value)) {
            runtimeError("A non-number value passed to average()");
            return NIL_VAL;
        }
        average = average + AS_NUMBER(value);
    }

    return NUMBER_VAL(average / argCount);
}

static Value floorNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("floor() takes 1 argument (%d given).", argCount);
        return NIL_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("A non-number value passed to floor()");
        return NIL_VAL;
    }


    return NUMBER_VAL(floor(AS_NUMBER(args[0])));
}

static Value roundNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("round() takes 1 argument (%d given).", argCount);
        return NIL_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("A non-number value passed to round()");
        return NIL_VAL;
    }


    return NUMBER_VAL(round(AS_NUMBER(args[0])));
}

static Value ceilNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("ceil() takes 1 argument (%d given).", argCount);
        return NIL_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("A non-number value passed to ceil()");
        return NIL_VAL;
    }


    return NUMBER_VAL(ceil(AS_NUMBER(args[0])));
}

static Value absNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("abs() takes 1 argument (%d given).", argCount);
        return NIL_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("A non-number value passed to abs()");
        return NIL_VAL;
    }

    double absValue = AS_NUMBER(args[0]);

    if (absValue < 0)
        return NUMBER_VAL(absValue * -1);
    return NUMBER_VAL(absValue);
}


static Value boolNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("bool() takes 1 argument (%d given).", argCount);
        return NIL_VAL;
    }

    return BOOL_VAL(!isFalsey(args[0]));
}


static Value inputNative(int argCount, Value *args) {
    if (argCount > 1) {
        runtimeError("input() takes 1 argument (%d given).", argCount);
        return NIL_VAL;
    }

    if (argCount != 0) {
        Value prompt = args[0];
        if (!IS_STRING(prompt)) {
            runtimeError("input() only takes a string argument");
            return NIL_VAL;
        }

        printf("%s", AS_CSTRING(prompt));
    }

    uint8_t len_max = 128;
    uint8_t current_size = len_max;

    char *line = malloc(len_max);

    if (line == NULL) {
        runtimeError("Memory error on input()!");
        return NIL_VAL;
    }

    int c = EOF;
    uint8_t i = 0;
    while ((c = getchar()) != '\n' && c != EOF) {
        line[i++] = (char) c;

        if (i == current_size) {
            current_size = i + len_max;
            line = realloc(line, current_size);
        }
    }

    line[i] = '\0';

    Value l = OBJ_VAL(copyString(line, strlen(line)));
    free(line);

    return l;
}

// Natives no return



static bool sleepNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("sleep() takes 1 argument (%d given)", argCount);
        return false;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("sleep() argument must be a number");
        return false;
    }

    double stopTime = AS_NUMBER(args[0]);

#ifdef _WIN32
    Sleep(stopTime * 1000);
#else
    sleep(stopTime);
#endif
    return true;
}

static bool printNative(int argCount, Value *args) {
    for (int i = 0; i < argCount; ++i) {
        Value value = args[i];
        printValue(value);
        printf("\n");
    }

    return true;
}

static bool assertNative(int argCount, Value *args) {
    Value value = args[0];

    if (!IS_BOOL(value)) {
        runtimeError("assert() only takes a boolean as an argument.", argCount);
        return false;
    }

    value = AS_BOOL(value);
    if (!value) {
        runtimeError("assert() was false!");
        return false;
    }

    return true;
}

static bool collectNative(int argCount, Value *args) {
    collectGarbage();
    return true;
}

// End of natives

void defineAllNatives() {
    char *nativeNames[] = {
            "clock",
            "sum",
            "min",
            "max",
            "average",
            "floor",
            "round",
            "ceil",
            "abs",
            "time",
            "len",
            "bool",
            "input",
            "number",
            "str",
            "type"
    };

    NativeFn nativeFunctions[] = {
            clockNative,
            sumNative,
            minNative,
            maxNative,
            averageNative,
            floorNative,
            roundNative,
            ceilNative,
            absNative,
            timeNative,
            lenNative,
            boolNative,
            inputNative,
            numberNative,
            strNative,
            typeNative
    };

    char *nativeVoidNames[] = {
            "sleep",
            "print",
            "assert",
            "collect"
    };

    NativeFnVoid nativeVoidFunctions[] = {
            sleepNative,
            printNative,
            assertNative,
            collectNative
    };


    for (uint8_t i = 0; i < sizeof(nativeNames) / sizeof(nativeNames[0]); ++i) {
        defineNative(nativeNames[i], nativeFunctions[i]);
    }

    for (uint8_t i = 0; i < sizeof(nativeVoidNames) / sizeof(nativeVoidNames[0]); ++i) {
        defineNativeVoid(nativeVoidNames[i], nativeVoidFunctions[i]);
    }
}
