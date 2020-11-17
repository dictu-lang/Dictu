#include "class.h"
#include "../vm.h"
#include "../memory.h"

static Value toString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "toString() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    char *valueString = classToString(args[0]);

    ObjString *string = copyString(vm, valueString, strlen(valueString));
    free(valueString);

    return OBJ_VAL(string);
}

void declareClassMethods(DictuVM *vm) {
    defineNative(vm, &vm->classMethods, "toString", toString);
}
