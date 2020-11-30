#include "hashlib.h"

static Value sha256(DictuVM *vm, int argCount, Value *args) {
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

static Value hmac(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 2 && argCount != 3) {
        runtimeError(vm, "hmac() takes 2 or 3 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0]) || !IS_STRING(args[1])) {
        runtimeError(vm, "Arguments passed to hmac() must be a string.");
        return EMPTY_VAL;
    }

    bool raw = false;

    if (argCount == 3) {
        if (!IS_BOOL(args[2])) {
            runtimeError(vm, "Optional argument passed to hmac() must be a boolean.");
            return EMPTY_VAL;
        }

        raw = AS_BOOL(args[2]);
    }

    ObjString *key = AS_STRING(args[0]);
    ObjString *string = AS_STRING(args[1]);

    uint8_t digest[32];
    struct tc_hmac_state_struct h;
    (void)tc_hmac_set_key(&h, (const uint8_t *) key->chars, key->length);
    tc_hmac_init(&h);
    tc_hmac_update(&h, string->chars, string->length);
    tc_hmac_final(digest, TC_SHA256_DIGEST_SIZE, &h);

    if (!raw) {
        char buffer[65];

        for (int i = 0; i < 32; i++ ) {
            sprintf( buffer + i * 2, "%02x", digest[i] );
        }

        return OBJ_VAL(copyString(vm, buffer, 64));
    }

    return OBJ_VAL(copyString(vm, (const char *)digest, 32));
}

static Value bcrypt(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1 && argCount != 2) {
        runtimeError(vm, "bcrypt() takes 1 or 2 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "Argument passed to bcrypt() must be a string.");
        return EMPTY_VAL;
    }

    int rounds = 8;

    if (argCount == 2) {
        if (!IS_NUMBER(args[1])) {
            runtimeError(vm, "Optional argument passed to bcrypt() must be a number.");
            return EMPTY_VAL;
        }

        rounds = AS_NUMBER(args[1]);
    }

    char *salt = bcrypt_gensalt(rounds);
    char *hash = bcrypt_pass(AS_CSTRING(args[0]), salt);

    return OBJ_VAL(copyString(vm, hash, strlen(hash)));
}

static Value bcryptVerify(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "bcryptVerify() takes 2 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0]) || !IS_STRING(args[1])) {
        runtimeError(vm, "Arguments passed to bcryptVerify() must be a string.");
        return EMPTY_VAL;
    }

    ObjString *stringA = AS_STRING(args[0]);
    ObjString *stringB = AS_STRING(args[1]);

    return BOOL_VAL(bcrypt_checkpass(stringA->chars, stringB->chars) == 0);
}

static Value verify(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "verify() takes 2 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0]) || !IS_STRING(args[1])) {
        runtimeError(vm, "Arguments passed to verify() must be a string.");
        return EMPTY_VAL;
    }

    ObjString *stringA = AS_STRING(args[0]);
    ObjString *stringB = AS_STRING(args[1]);

    if (stringA->length != stringB->length) {
        return FALSE_VAL;
    }

    return BOOL_VAL(_compare((const uint8_t *) stringA->chars, (const uint8_t *) stringB->chars, stringA->length) == 0);
}

ObjModule *createHashlibModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Hashlib", 7);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Http methods
     */
    defineNative(vm, &module->values, "strerror", strerrorNative);
    defineNative(vm, &module->values, "sha256", sha256);
    defineNative(vm, &module->values, "hmac", hmac);
    defineNative(vm, &module->values, "bcrypt", bcrypt);
    defineNative(vm, &module->values, "verify", verify);
    defineNative(vm, &module->values, "bcryptVerify", bcryptVerify);

    /**
     * Define Http properties
     */
    defineNativeProperty(vm, &module->values, "errno", NUMBER_VAL(0));
    pop(vm);
    pop(vm);

    return module;
}