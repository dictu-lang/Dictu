#include "object.h"

#include "object-source.h"

static Value objectGetClassRefImpl(DictuVM *vm, int argCount, Value *args, bool internal) {
    if (argCount != 1) {
        runtimeError(vm, "getClassRef() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        int valLength = 0;
        char *val = valueTypeToString(vm, args[0], &valLength);
        runtimeError(vm, "getClassRef() argument must be a string, got '%s'.", val);
        FREE_ARRAY(vm, char, val, valLength + 1);
        return EMPTY_VAL;
    }

    ObjString *classString = AS_STRING(args[0]);

    Value klass;
    CallFrame *frame;
    if (internal) {
        // -2 as we want to go to the callee site, not the Object.du code
        frame = &vm->frames[vm->frameCount - 2];
    } else {
        frame = &vm->frames[vm->frameCount - 1];
    }

    if (tableGet(&frame->closure->function->module->values, classString, &klass) && IS_CLASS(klass)) {
        return newResultSuccess(vm, klass);
    }

    char *error = ALLOCATE(vm, char, classString->length + 26);
    memcpy(error, classString->chars, classString->length);
    memcpy(error + classString->length, " class could not be found", 25);
    error[classString->length + 25] = '\0';

    Value result = newResultError(vm, error);

    FREE_ARRAY(vm, char, error, classString->length + 26);

    return result;
}

static Value objectGetClassRef(DictuVM *vm, int argCount, Value *args) {
    return objectGetClassRefImpl(vm, argCount, args, false);
}

static Value objectGetClassRefInternal(DictuVM *vm, int argCount, Value *args) {
    return objectGetClassRefImpl(vm, argCount, args, true);
}

static Value objectHash(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "hash() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    char str[21];
    sprintf(str, "%llu", (unsigned long long)args[0]);
    return OBJ_VAL(copyString(vm, (char *)str, 21));
}

static Value objectPrettyPrint(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1 && argCount != 2) {
        runtimeError(vm, "prettyPrint() takes 1 or 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Value out = stringify(vm, argCount, args);
    ObjResult *res = AS_RESULT(out);
    if (res->status == ERR) {
        runtimeError(vm, AS_CSTRING(res->value));
        return EMPTY_VAL;
    }
    
    printValue(res->value);
    printf("\n");

    return NIL_VAL;
}

Value createObjectModule(DictuVM *vm) {
    ObjClosure *closure = compileModuleToClosure(vm, "Object", DICTU_OBJECT_SOURCE);

    if (closure == NULL) {
        return EMPTY_VAL;
    }

    push(vm, OBJ_VAL(closure));

    /**
     * Define Object methods
     */
    defineNative(vm, &closure->function->module->values, "__getClassRef", objectGetClassRefInternal);
    defineNative(vm, &closure->function->module->values, "getClassRef", objectGetClassRef);
    defineNative(vm, &closure->function->module->values, "hash", objectHash);

    defineNative(vm, &closure->function->module->values, "prettyPrint", objectPrettyPrint);

    pop(vm);

    return OBJ_VAL(closure);
}