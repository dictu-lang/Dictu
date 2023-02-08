#include "bigint.h"

#ifndef DISABLE_BIGINT
#include <tommath.h>

#define AS_BIGINT(abstract) ((BigIntData*) ((abstract)->data))

static Value compare(DictuVM *vm, int argCount, Value *args);
static Value arithNeg(DictuVM *vm, int argCount, Value *args);
static Value arithAbs(DictuVM *vm, int argCount, Value *args);
static Value arithSqr(DictuVM *vm, int argCount, Value *args);
static Value bitwiseAnd(DictuVM *vm, int argCount, Value *args);
static Value bitwiseOr(DictuVM *vm, int argCount, Value *args);
static Value bitwiseXor(DictuVM *vm, int argCount, Value *args);
static Value arithAdd(DictuVM *vm, int argCount, Value *args);
static Value arithSub(DictuVM *vm, int argCount, Value *args);
static Value arithMul(DictuVM *vm, int argCount, Value *args);
static Value arithDiv(DictuVM *vm, int argCount, Value *args);
static Value arithMod(DictuVM *vm, int argCount, Value *args);

typedef struct {
    mp_int value;
} BigIntData;

static void freeBigInt(DictuVM *vm, ObjAbstract *abstract)
{
    UNUSED(vm);
    mp_clear(&AS_BIGINT(abstract)->value);
}

static char *bigIntToString(ObjAbstract *abstract) {
    int len = 0;
    int res = mp_radix_size(&AS_BIGINT(abstract)->value, 10, &len);
    UNUSED(res);
    char *str = malloc(len);
    res = mp_to_radix(&AS_BIGINT(abstract)->value, str, len, NULL, 10);
    return str;
}

static bool isLong(Value val) {
    double num = AS_NUMBER(val);
    return num == ((double) (long) num);
}

static bool isIntString(Value val) {
    ObjString *str = AS_STRING(val);

    for (int i = 0; i < str->length; i++) {
        if (!isdigit(str->chars[i])) {
            return false;
        }
    }

    return true;
}

static bool isBigInt(ObjAbstract* abstract) {
    UNUSED(abstract);
    return abstract->func == &freeBigInt;
}

static bool isBigIntValue(Value val) {
    if (!IS_ABSTRACT(val)) {
        return false;
    }

    return isBigInt((ObjAbstract*) AS_ABSTRACT(val));
}

static BigIntData *allocBigInt(DictuVM *vm) {
    BigIntData *bigint = ALLOCATE(vm, BigIntData, 1);
    int res = mp_init(&bigint->value);
    UNUSED(res);
    return bigint;
}

static BigIntData *cloneBigInt(DictuVM *vm, BigIntData* toClone) {
    BigIntData *bigint = allocBigInt(vm);
    int res = mp_init_copy(&bigint->value, &toClone->value);
    UNUSED(res);
    return bigint;
}

static BigIntData *bigIntFromIntString(DictuVM *vm, Value strValue) {
    if (!isIntString(strValue)) {
        runtimeError(vm, "bigint() string argument must only contain digits ('%s' given)", AS_STRING(strValue)->chars);
        return NULL;
    }

    ObjString* str = AS_STRING(strValue);
    BigIntData *bigint = allocBigInt(vm);
    int res = mp_read_radix(&bigint->value, str->chars, 10);
    UNUSED(res);
    return bigint;
}

static BigIntData *bigIntFromLong(DictuVM *vm, Value numValue) {
    if (!isLong(numValue)) {
        runtimeError(vm, "bigint() number argument must be an integer ('%f' given)", AS_NUMBER(numValue));
        return NULL;
    }

    long num = (long) AS_NUMBER(numValue);
    BigIntData *bigint = allocBigInt(vm);
    mp_set_l(&bigint->value, num);
    return bigint;
}

static Value newBigIntValue(DictuVM *vm, BigIntData *bigint) {
    if (bigint == NULL) {
        return EMPTY_VAL;
    }

    ObjAbstract *abstract = newAbstract(vm, &freeBigInt, &bigIntToString);
    push(vm, OBJ_VAL(abstract));
    abstract->data = bigint;

    /**
     * Setup BigInt object methods
     */
    defineNative(vm, &abstract->values, "compare",     &compare);
    defineNative(vm, &abstract->values, "negate",      &arithNeg);
    defineNative(vm, &abstract->values, "absoluteVal", &arithAbs);
    defineNative(vm, &abstract->values, "squared",     &arithSqr);
    defineNative(vm, &abstract->values, "and",         &bitwiseAnd);
    defineNative(vm, &abstract->values, "or",          &bitwiseOr);
    defineNative(vm, &abstract->values, "xor",         &bitwiseXor);
    defineNative(vm, &abstract->values, "add",         &arithAdd);
    defineNative(vm, &abstract->values, "subtract",    &arithSub);
    defineNative(vm, &abstract->values, "multiply",    &arithMul);
    defineNative(vm, &abstract->values, "divide",      &arithDiv);
    defineNative(vm, &abstract->values, "modulo",      &arithMod);
    pop(vm);

    return OBJ_VAL(abstract);
}

static Value newBigInt(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "bigint() takes 1 argument (%0d given)", argCount);
        return EMPTY_VAL;
    }

    if (IS_NUMBER(args[0])) {
        return newBigIntValue(vm, bigIntFromLong(vm, args[0]));
    } else if (IS_STRING(args[0])) {
        return newBigIntValue(vm, bigIntFromIntString(vm, args[0]));
    } else if (isBigIntValue(args[0])) {
        return newBigIntValue(vm, cloneBigInt(vm, AS_ABSTRACT(args[0])->data));
    }

    runtimeError(vm, "bigint() argument must be an integral number, string or bigint");
    return EMPTY_VAL;
}

static Value applyOp1Arg(DictuVM *vm, int argCount, Value *args, const char* fname, int (*op)(const mp_int*, const mp_int*)) {
    if (argCount != 1) {
        runtimeError(vm, "%s() takes 1 argument (%0d given)", fname, argCount);
        return EMPTY_VAL;
    }

    if (!isBigIntValue(args[1])) {
        runtimeError(vm, "%s() argument must be a bigint", fname, argCount);
        return EMPTY_VAL;
    }

    int res = op(&AS_BIGINT(AS_ABSTRACT(args[0]))->value, &AS_BIGINT(AS_ABSTRACT(args[1]))->value);
    return NUMBER_VAL(res);
}

static Value applyOp0ArgAndReturn(DictuVM *vm, int argCount, Value *args, const char* fname, int (*op)(const mp_int*, mp_int*)) {
    if (argCount != 0) {
        runtimeError(vm, "%s() takes no arguments (%0d given)", fname, argCount);
        return EMPTY_VAL;
    }

    BigIntData *bigint = allocBigInt(vm);
    int res = op(&AS_BIGINT(AS_ABSTRACT(args[0]))->value, &bigint->value);
    UNUSED(res);
    return newBigIntValue(vm, bigint);
}

static Value applyOp1ArgAndReturn(DictuVM *vm, int argCount, Value *args, const char* fname, int (*op)(const mp_int*, const mp_int*, mp_int*)) {
    if (argCount != 1) {
        runtimeError(vm, "%s() takes 1 argument (%0d given)", fname, argCount);
        return EMPTY_VAL;
    }

    if (!isBigIntValue(args[1])) {
        runtimeError(vm, "%s() argument must be a bigint", fname, argCount);
        return EMPTY_VAL;
    }

    BigIntData *bigint = allocBigInt(vm);
    int res = op(&AS_BIGINT(AS_ABSTRACT(args[0]))->value, &AS_BIGINT(AS_ABSTRACT(args[1]))->value, &bigint->value);
    UNUSED(res);
    return newBigIntValue(vm, bigint);
}

static Value compare(DictuVM *vm, int argCount, Value *args) {
    return applyOp1Arg(vm, argCount, args, "compare", &mp_cmp);
}

static Value arithNeg(DictuVM *vm, int argCount, Value *args) {
    return applyOp0ArgAndReturn(vm, argCount, args, "negate", &mp_neg);
}

static Value arithAbs(DictuVM *vm, int argCount, Value *args) {
    return applyOp0ArgAndReturn(vm, argCount, args, "absoluteVal", &mp_abs);
}

static Value arithSqr(DictuVM *vm, int argCount, Value *args) {
    return applyOp0ArgAndReturn(vm, argCount, args, "squared", &mp_sqr);
}

static Value bitwiseAnd(DictuVM *vm, int argCount, Value *args) {
    return applyOp1ArgAndReturn(vm, argCount, args, "and", &mp_and);
}

static Value bitwiseOr(DictuVM *vm, int argCount, Value *args) {
    return applyOp1ArgAndReturn(vm, argCount, args, "or", &mp_or);
}

static Value bitwiseXor(DictuVM *vm, int argCount, Value *args) {
    return applyOp1ArgAndReturn(vm, argCount, args, "xor", &mp_xor);
}

static Value arithAdd(DictuVM *vm, int argCount, Value *args) {
    return applyOp1ArgAndReturn(vm, argCount, args, "add", &mp_add);
}

static Value arithSub(DictuVM *vm, int argCount, Value *args) {
    return applyOp1ArgAndReturn(vm, argCount, args, "subtract", &mp_sub);
}

static Value arithMul(DictuVM *vm, int argCount, Value *args) {
    return applyOp1ArgAndReturn(vm, argCount, args, "multiply", &mp_mul);
}

static int mp_div_only(const mp_int* a, const mp_int* b, mp_int* c) {
    return mp_div(a, b, c, NULL);
}

static int mp_mod_only(const mp_int* a, const mp_int* b, mp_int* c) {
    return mp_div(a, b, NULL, c);
}

static Value arithDiv(DictuVM *vm, int argCount, Value *args) {
    return applyOp1ArgAndReturn(vm, argCount, args, "divide", &mp_div_only);
}

static Value arithMod(DictuVM *vm, int argCount, Value *args) {
    return applyOp1ArgAndReturn(vm, argCount, args, "modulo", &mp_mod_only);
}
#endif

Value createBigIntModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "BigInt", 6);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define BigInt methods
     */
#ifndef DISABLE_BIGINT
    defineNative(vm, &module->values, "bigint", newBigInt);
#endif

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
