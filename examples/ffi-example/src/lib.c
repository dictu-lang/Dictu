#include "dictu-include.h"

Value dictu_ffi_test(DictuVM *vm, int argCount, Value *args) {
    if(argCount != 2 || !IS_NUMBER(args[0]) || !IS_NUMBER(args[1])){
        return NIL_VAL;
    }
    double a = AS_NUMBER(args[0]);
    double b = AS_NUMBER(args[1]);
    return NUMBER_VAL(a+b);
}
Value dictu_ffi_test_str(DictuVM *vm, int argCount, Value *args) {
    return OBJ_VAL(copyString(vm, "Hello world", 11));
}