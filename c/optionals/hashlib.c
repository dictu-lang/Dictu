#include "hashlib.h"

static Value sha256(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "sha256() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "Argument passed to sha256() must be a string.");
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);

    uint8_t digest[32];
    struct tc_sha256_state_struct s;

    if (!tc_sha256_init(&s)) {
        return NIL_VAL;
    }

    if (!tc_sha256_update(&s, (const uint8_t *) string->chars, string->length)) {
        return NIL_VAL;
    }

    if (!tc_sha256_final(digest, &s)) {
        return NIL_VAL;
    }

    char buffer[65];

    for (int i = 0; i < 32; i++ ) {
        sprintf( buffer + i * 2, "%02x", digest[i] );
    }

    return OBJ_VAL(copyString(vm, buffer, 64));
}

ObjModule *createHashlibModule(VM *vm) {
    ObjString *name = copyString(vm, "Hashlib", 7);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Http methods
     */
    defineNative(vm, &module->values, "strerror", strerrorNative);
    defineNative(vm, &module->values, "sha256", sha256);

    /**
     * Define Http properties
     */
    defineNativeProperty(vm, &module->values, "errno", NUMBER_VAL(0));
    pop(vm);
    pop(vm);

    return module;
}