#include "number.h"
#include "../vm.h"

#include <stdlib.h>

static Value toStringNumber(VM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "toString() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    double number = AS_NUMBER(args[0]);
    int numberStringLength = snprintf(NULL, 0, "%.15g", number) + 1;
    
    #ifdef NO_VLA
    char *numberString = malloc(numberStringLength);
    if (numberString == NULL) {
        runtimeError(vm, "Memory error on toString()!");
        return EMPTY_VAL;
    }
    #else
    char numberString[numberStringLength];
    #endif
    
    snprintf(numberString, numberStringLength, "%.15g", number);
    Value value = OBJ_VAL(copyString(vm, numberString, numberStringLength - 1));

    #ifdef NO_VLA
    free(numberString);
    #endif

    return value;
}

void declareNumberMethods(VM *vm) {
    defineNative(vm, &vm->numberMethods, "toString", toStringNumber);
    defineNative(vm, &vm->numberMethods, "toBool", boolNative); // Defined in util
}
