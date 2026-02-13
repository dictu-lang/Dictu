#include "uuid.h"

#ifndef _WIN32
static uuid_t uuid;

#define UUID_STRING_LEN 36 // Note(Liz3): the strlen() of a UUID is 36, but the uuid_unparse_lower call expects
                           // a buffer with space for the null byte, but since copyString wants the strlen we
                           // keep the define to 36 and allocate +1.

static Value uuidGenerateNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "generate() doesn't take any arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    uuid_clear(uuid);

    uuid_generate(uuid);

    if (uuid_is_null(uuid)) {
        return newResultError(vm, "error: failed to generate uuid");
    }

    char out[UUID_STRING_LEN+1] = {};
    uuid_unparse_lower(uuid, out);

    return newResultSuccess(vm, OBJ_VAL(copyString(vm, out, UUID_STRING_LEN)));
}

static Value uuidGenRandomNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "generateRandom() doesn't take any arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    uuid_clear(uuid);

    uuid_generate_random(uuid);

    if (uuid_is_null(uuid)) {
        return newResultError(vm, "error: failed to generate uuid");
    }

    char out[UUID_STRING_LEN+1] = {};
    uuid_unparse_lower(uuid, out);

    return newResultSuccess(vm, OBJ_VAL(copyString(vm, out, UUID_STRING_LEN)));
}

static Value uuidGenTimeNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "generateTime() doesn't take any arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    uuid_clear(uuid);

    uuid_generate_time(uuid);

    if (uuid_is_null(uuid)) {
        return newResultError(vm, "error: failed to generate uuid");
    }

    char out[UUID_STRING_LEN+1] = {};
    uuid_unparse_lower(uuid, out);

    return newResultSuccess(vm, OBJ_VAL(copyString(vm, out, UUID_STRING_LEN)));
}
#endif

Value createUuidModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "UUID", 4);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define UUID methods
     */
#ifndef _WIN32
    defineNative(vm, &module->values, "generate", uuidGenerateNative);
    defineNative(vm, &module->values, "generateRandom", uuidGenRandomNative);
    defineNative(vm, &module->values, "generateTime", uuidGenTimeNative);
#endif
    /**
     * Define UUID properties
     */

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
