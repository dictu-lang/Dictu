#include "random.h"

static Value randomRandom(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);
    if (argCount > 0) {
        runtimeError(vm, "random() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    int high = 1;
    int low = 0;
    double random_double = ((double)rand() * (high - low)) / (double)RAND_MAX + low;
    return NUMBER_VAL(random_double);
}

static Value randomRange(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "range() takes 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1])) {
        runtimeError(vm, "range() arguments must be numbers");
        return EMPTY_VAL;
    }

    int upper = AS_NUMBER(args[1]);
    int lower = AS_NUMBER(args[0]);
    int random_val = (rand() % (upper - lower + 1)) + lower;
    return NUMBER_VAL(random_val);
}

static Value randomSelect(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "select() takes one argument (%d provided)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_LIST(args[0])) {
        runtimeError(vm, "select() argument must be a list");
        return EMPTY_VAL;
    }

    ObjList *list = AS_LIST(args[0]);
    argCount = list->values.count;
    args = list->values.values;

    int index = rand() % argCount;
    return args[index];
}

Value createRandomModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Random", 6);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    srand(time(NULL));

    /**
     * Define Random methods
     */
    defineNative(vm, &module->values, "random", randomRandom);
    defineNative(vm, &module->values, "range", randomRange);
    defineNative(vm, &module->values, "select", randomSelect);

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}