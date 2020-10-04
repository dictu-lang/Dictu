#include "random.h"

static Value randomRandom(VM * vm, int argCount, Value *args) {
    args = args;  // FIXME: Can't get -Werror to surpress not using args
    if (argCount > 0) {
        runtimeError(vm, "random() takes 0 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }
    return NUMBER_VAL(rand() % 2);
}


ObjModule *createRandomClass(VM *vm) {
    ObjString *name = copyString(vm, "Random", 6);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Random methods
     */
    defineNative(vm, &module->values, "random", randomRandom);
    // TODO: Define randomRange and randomSelect

    pop(vm);
    pop(vm);

    return module;
}