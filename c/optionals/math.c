#include "math.h"
#include "../vm.h"
#include <math.h>

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
            return EMPTY_VAL;
        }
        average = average + AS_NUMBER(value);
    }

    return NUMBER_VAL(average / argCount);
}

static Value floorNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("floor() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("A non-number value passed to floor()");
        return EMPTY_VAL;
    }


    return NUMBER_VAL(floor(AS_NUMBER(args[0])));
}

static Value roundNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("round() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("A non-number value passed to round()");
        return EMPTY_VAL;
    }


    return NUMBER_VAL(round(AS_NUMBER(args[0])));
}

static Value ceilNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("ceil() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("A non-number value passed to ceil()");
        return EMPTY_VAL;
    }


    return NUMBER_VAL(ceil(AS_NUMBER(args[0])));
}

static Value absNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("abs() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("A non-number value passed to abs()");
        return EMPTY_VAL;
    }

    double absValue = AS_NUMBER(args[0]);

    if (absValue < 0)
        return NUMBER_VAL(absValue * -1);
    return NUMBER_VAL(absValue);
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
            return EMPTY_VAL;
        }
        sum = sum + AS_NUMBER(value);
    }

    return NUMBER_VAL(sum);
}

static Value minNative(int argCount, Value *args) {
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
            return EMPTY_VAL;
        }

        double current = AS_NUMBER(value);

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
            return EMPTY_VAL;
        }

        double current = AS_NUMBER(value);

        if (maximum < current) {
            maximum = current;
        }
    }

    return NUMBER_VAL(maximum);
}

void createMathsClass() {
    ObjString *name = copyString("Math", 4);
    push(OBJ_VAL(name));
    ObjClassNative *klass = newClassNative(name);
    push(OBJ_VAL(klass));

    /**
     * Define Math methods
     */
    defineNativeMethod(klass, "average", averageNative);
    defineNativeMethod(klass, "floor", floorNative);
    defineNativeMethod(klass, "round", roundNative);
    defineNativeMethod(klass, "ceil", ceilNative);
    defineNativeMethod(klass, "abs", absNative);
    defineNativeMethod(klass, "max", maxNative);
    defineNativeMethod(klass, "min", minNative);
    defineNativeMethod(klass, "sum", sumNative);

    tableSet(&vm.globals, name, OBJ_VAL(klass));
    pop();
    pop();
}