#include <dictu_ffi_include.h>

Value dictu_ffi_test(DictuVM *vm, int argCount, Value *args) {
    if(argCount != 2 || !IS_NUMBER(args[0]) || !IS_NUMBER(args[1])){
        return NIL_VAL;
    }
    double a = AS_NUMBER(args[0]);
    double b = AS_NUMBER(args[1]);
    return NUMBER_VAL(a+b);
}
Value dictu_ffi_test_str(DictuVM *vm, int argCount, Value *args) {
    return OBJ_VAL(copyString(vm, "Hello From Dictu FFI module!", 28));
}

Value dictu_ffi_test_callback(DictuVM *vm, int argCount, Value *args) {
    if(argCount < 1)
        return NIL_VAL;
    Value* funcs = args+1;
    return callFunction(vm, args[0], argCount-1, funcs);
}

int dictu_ffi_init(DictuVM *vm, Table *method_table) {
  defineNative(vm, method_table, "dictuFFITestAdd", dictu_ffi_test);
  defineNative(vm, method_table, "dictuFFITestStr", dictu_ffi_test_str);
  defineNative(vm, method_table, "dictuFFITestCallback", dictu_ffi_test_callback);
  defineNativeProperty(
    vm, method_table, "test",
        OBJ_VAL(copyString(vm, "Dictu!", 6)));
  return 0;
}
