#include "math.h"
#include "../vm/vm.h"
#include <math.h>

static Value averageNative(DictuVM *vm, int argCount, Value *args) {
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
            runtimeError(vm, "A non-number value passed to average()");
            return EMPTY_VAL;
        }
        average = average + AS_NUMBER(value);
    }

    return NUMBER_VAL(average / argCount);
}

static Value floorNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "floor() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError(vm, "A non-number value passed to floor()");
        return EMPTY_VAL;
    }

    return NUMBER_VAL(floor(AS_NUMBER(args[0])));
}

static Value roundNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "round() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError(vm, "A non-number value passed to round()");
        return EMPTY_VAL;
    }

    return NUMBER_VAL(round(AS_NUMBER(args[0])));
}

static Value ceilNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "ceil() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError(vm, "A non-number value passed to ceil()");
        return EMPTY_VAL;
    }

    return NUMBER_VAL(ceil(AS_NUMBER(args[0])));
}

static Value absNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "abs() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError(vm, "A non-number value passed to abs()");
        return EMPTY_VAL;
    }

    double absValue = AS_NUMBER(args[0]);

    if (absValue < 0)
        return NUMBER_VAL(absValue * -1);

    return NUMBER_VAL(absValue);
}

static Value maxNative(DictuVM *vm, int argCount, Value *args) {
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
            runtimeError(vm, "A non-number value passed to max()");
            return EMPTY_VAL;
        }

        double current = AS_NUMBER(value);

        if (maximum < current) {
            maximum = current;
        }
    }

    return NUMBER_VAL(maximum);
}

static Value minNative(DictuVM *vm, int argCount, Value *args) {
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
            runtimeError(vm, "A non-number value passed to min()");
            return EMPTY_VAL;
        }

        double current = AS_NUMBER(value);

        if (minimum > current) {
            minimum = current;
        }
    }

    return NUMBER_VAL(minimum);
}

static Value sumNative(DictuVM *vm, int argCount, Value *args) {
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
            runtimeError(vm, "A non-number value passed to sum()");
            return EMPTY_VAL;
        }
        sum = sum + AS_NUMBER(value);
    }

    return NUMBER_VAL(sum);
}

static Value sqrtNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "sqrt() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError(vm, "A non-number value passed to sqrt()");
        return EMPTY_VAL;
    }

    return NUMBER_VAL(sqrt(AS_NUMBER(args[0])));
}

static Value sinNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "sin() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError(vm, "A non-number value passed to sin()");
        return EMPTY_VAL;
    }

    return NUMBER_VAL(sin(AS_NUMBER(args[0])));
}

static Value cosNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "cos() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError(vm, "A non-number value passed to cos()");
        return EMPTY_VAL;
    }

    return NUMBER_VAL(cos(AS_NUMBER(args[0])));
}

static Value tanNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "tan() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError(vm, "A non-number value passed to tan()");
        return EMPTY_VAL;
    }

    return NUMBER_VAL(tan(AS_NUMBER(args[0])));
}

static long long gcd(long long a, long long b) {
    long long r;
    while (b > 0) {
        r = a % b;
        a = b;
        b = r;
    }
    return a;
}

static Value gcdNative(DictuVM *vm, int argCount, Value *args) {
    char* argCountError = "gcd() requires 2 or more arguments (%d given).";
    char* nonNumberError = "gcd() argument at index %d is not a number";
    char* notWholeError = "gcd() argument (%f) at index %d is not a whole number";

    if (argCount == 1 && IS_LIST(args[0])) {
        argCountError = "List passed to gcd() must have 2 or more elements (%d given).";
        nonNumberError = "The element at index %d of the list passed to gcd() is not a number";
        notWholeError = "The element (%f) at index %d of the list passed to gcd() is not a whole number";
        ObjList *list = AS_LIST(args[0]);
        argCount = list->values.count;
        args = list->values.values;
    }

    if (argCount < 2) {
        runtimeError(vm, argCountError, argCount);
        return EMPTY_VAL;
    }

    for (int i = 0; i < argCount; ++i)
        if (!IS_NUMBER(args[i])) {
            runtimeError(vm, nonNumberError, i);
            return EMPTY_VAL;
        }

    double* as_doubles = ALLOCATE(vm, double, argCount);
    for (int i = 0; i < argCount; ++i) {
        as_doubles[i] = AS_NUMBER(args[i]);
        if (fabs(round(as_doubles[i]) - as_doubles[i]) > FLOAT_TOLERANCE) {
            runtimeError(vm, notWholeError, as_doubles[i], i);
            FREE_ARRAY(vm, double, as_doubles, argCount);
            return EMPTY_VAL;
        }
    }

    long long* as_longlongs = ALLOCATE(vm, long long, argCount);
    for (int i = 0; i < argCount; ++i) as_longlongs[i] = round(as_doubles[i]);

    long long result = as_longlongs[0];
    for (int i = 1; i < argCount; ++i) result = gcd(result, as_longlongs[i]);

    FREE_ARRAY(vm, double, as_doubles, argCount);
    FREE_ARRAY(vm, long long, as_longlongs, argCount);
    return NUMBER_VAL(result);
}

long long lcm(long long a, long long b) {
    return (a * b) / gcd(a, b);
}

static Value lcmNative(DictuVM *vm, int argCount, Value *args) {
    char* argCountError = "lcm() requires 2 or more arguments (%d given).";
    char* nonNumberError = "lcm() argument at index %d is not a number";
    char* notWholeError = "lcm() argument (%f) at index %d is not a whole number";

    if (argCount == 1 && IS_LIST(args[0])) {
        argCountError = "List passed to lcm() must have 2 or more elements (%d given).";
        nonNumberError = "The element at index %d of the list passed to lcm() is not a number";
        notWholeError = "The element (%f) at index %d of the list passed to lcm() is not a whole number";
        ObjList *list = AS_LIST(args[0]);
        argCount = list->values.count;
        args = list->values.values;
    }

    if (argCount < 2) {
        runtimeError(vm, argCountError, argCount);
        return EMPTY_VAL;
    }

    for (int i = 0; i < argCount; ++i)
        if (!IS_NUMBER(args[i])) {
            runtimeError(vm, nonNumberError, i);
            return EMPTY_VAL;
        }

    double* as_doubles = ALLOCATE(vm, double, argCount);
    for (int i = 0; i < argCount; ++i) {
        as_doubles[i] = AS_NUMBER(args[i]);
        if (fabs(round(as_doubles[i]) - as_doubles[i]) > FLOAT_TOLERANCE) {
            runtimeError(vm, notWholeError, as_doubles[i], i);
            FREE_ARRAY(vm, double, as_doubles, argCount);
            return EMPTY_VAL;
        }
    }

    long long* as_longlongs = ALLOCATE(vm, long long, argCount);
    for (int i = 0; i < argCount; ++i) as_longlongs[i] = round(as_doubles[i]);

    long long result = as_longlongs[0];
    for (int i = 1; i < argCount; ++i) result = lcm(result, as_longlongs[i]);

    FREE_ARRAY(vm, double, as_doubles, argCount);
    FREE_ARRAY(vm, long long, as_longlongs, argCount);
    return NUMBER_VAL(result);
}

ObjModule *createMathsModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Math", 4);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Math methods
     */
    defineNative(vm, &module->values, "average", averageNative);
    defineNative(vm, &module->values, "floor", floorNative);
    defineNative(vm, &module->values, "round", roundNative);
    defineNative(vm, &module->values, "ceil", ceilNative);
    defineNative(vm, &module->values, "abs", absNative);
    defineNative(vm, &module->values, "max", maxNative);
    defineNative(vm, &module->values, "min", minNative);
    defineNative(vm, &module->values, "sum", sumNative);
    defineNative(vm, &module->values, "sqrt", sqrtNative);
    defineNative(vm, &module->values, "sin", sinNative);
    defineNative(vm, &module->values, "cos", cosNative);
    defineNative(vm, &module->values, "tan", tanNative);
    defineNative(vm, &module->values, "gcd", gcdNative);
    defineNative(vm, &module->values, "lcm", lcmNative);

    /**
     * Define Math properties
     */
    defineNativeProperty(vm, &module->values, "PI", NUMBER_VAL(3.14159265358979));
    defineNativeProperty(vm, &module->values, "e", NUMBER_VAL(2.71828182845905));
    pop(vm);
    pop(vm);

    return module;
}
