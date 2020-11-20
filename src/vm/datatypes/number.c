#include "number.h"
#include "../memory.h"

static Value toStringNumber(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "toString() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    double number = AS_NUMBER(args[0]);
    int numberStringLength = snprintf(NULL, 0, "%.15g", number) + 1;
    
    char *numberString = ALLOCATE(vm, char, numberStringLength);

    if (numberString == NULL) {
        runtimeError(vm, "Memory error on toString()!");
        return EMPTY_VAL;
    }
    
    snprintf(numberString, numberStringLength, "%.15g", number);
    Value newString = OBJ_VAL(copyString(vm, numberString, numberStringLength - 1));
    FREE_ARRAY(vm, char, numberString, numberStringLength);

    return newString;
}

void declareNumberMethods(DictuVM *vm) {
    defineNative(vm, &vm->numberMethods, "toString", toStringNumber);
    defineNative(vm, &vm->numberMethods, "toBool", boolNative); // Defined in util
}
