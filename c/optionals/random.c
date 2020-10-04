#include "random.h"

static Value randomRandom(VM *vm, int argCount, Value *args) {
    args = args;  // FIXME: Can't get -Werror to surpress not using args
    if (argCount > 0) {
        runtimeError(vm, "random() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }
    return NUMBER_VAL(rand() % 2);
}

static Value randomRange(VM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "range() takes 2 arguments (%0d given)", argCount);
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

// static Value randomSelect(VM *vm, int argCount, Value *args) {

// }

ObjModule *createRandomClass(VM *vm) {
    ObjString *name = copyString(vm, "Random", 6);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Random methods
     */
    defineNative(vm, &module->values, "random", randomRandom);
    defineNative(vm, &module->values, "range", randomRange);
    // defineNative(vm, &module->values, "random", randomSelect);

    pop(vm);
    pop(vm);

    return module;
}