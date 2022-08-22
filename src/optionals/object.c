#include "object.h"

static Value objectCreateFrom(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "from() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "from() argument must be a string");
        return EMPTY_VAL;
    }

    ObjString *classString = AS_STRING(args[0]);

    Value klass;
    CallFrame *frame = &vm->frames[vm->frameCount - 1];

    if (tableGet(&frame->closure->function->module->values, classString, &klass) && IS_CLASS(klass)) {
        ObjInstance *instance = newInstance(vm, AS_CLASS(klass));
        return OBJ_VAL(instance);
    }

    return NIL_VAL;
}

Value createObjectModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Object", 6);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    srand(time(NULL));

    /**
     * Define Object methods
     */
    defineNative(vm, &module->values, "createFrom", objectCreateFrom);

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}