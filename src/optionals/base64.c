#include "base64.h"

static Value encode(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "encode() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "encode() argument must be a string");
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);

    int size = b64e_size(string->length) + 1;
    char *buffer = ALLOCATE(vm, char, size);

    int actualSize = b64_encode((unsigned char*)string->chars, string->length, (unsigned char*)buffer);

    ObjString *encodedString = copyString(vm, buffer, actualSize);
    FREE_ARRAY(vm, char, buffer, size);

    return OBJ_VAL(encodedString);
}

static Value decode(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "encode() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "decode() argument must be a string");
        return EMPTY_VAL;
    }

    ObjString *encodedString = AS_STRING(args[0]);

    int size = b64d_size(encodedString->length) + 1;
    char *buffer = ALLOCATE(vm, char, size);

    int actualSize = b64_decode((unsigned char*)encodedString->chars, encodedString->length, (unsigned char*)buffer);

    ObjString *string = copyString(vm, buffer, actualSize);
    FREE_ARRAY(vm, char, buffer, size);

    return OBJ_VAL(string);
}

Value createBase64Module(DictuVM *vm) {
    ObjString *name = copyString(vm, "Base64", 6);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Base64 methods
     */
    defineNative(vm, &module->values, "encode", encode);
    defineNative(vm, &module->values, "decode", decode);

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}