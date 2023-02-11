#include "bigint.h"

#include "bigint/bigint.h"

#define AS_BIGINT(abstract) ((BigIntData*) ((abstract)->data))

#define DECL_OP(name) \
    static Value name(DictuVM *vm, int argCount, Value *args)

#define APPLY_OP(name, optype, func) \
    DECL_OP(name) { \
        return apply##optype(vm, argCount, args, func); \
    }

typedef struct {
    bigint value;
} BigIntData;

DECL_OP(compare);
DECL_OP(arithNeg);
DECL_OP(arithAbs);
DECL_OP(bitwiseAnd);
DECL_OP(bitwiseOr);
DECL_OP(bitwiseXor);
DECL_OP(arithAdd);
DECL_OP(arithSub);
DECL_OP(arithMul);
DECL_OP(arithDiv);
DECL_OP(arithMod);

static void freeBigInt(DictuVM *vm, ObjAbstract *abstract) {
    bigint_free(&AS_BIGINT(abstract)->value);
    FREE(vm, BigIntData, AS_BIGINT(abstract));
}

static char *bigIntToString(ObjAbstract *abstract) {
    int len = bigint_write_size(&AS_BIGINT(abstract)->value, 10);
    char *str = malloc(len);
    return bigint_write(str, len, &AS_BIGINT(abstract)->value);
}

static bool isLong(Value val) {
    double num = AS_NUMBER(val);
    return num == ((double) (long) num);
}

static bool isIntString(Value val) {
    ObjString *str = AS_STRING(val);
    int i = 0;

    if (str->chars[i] == '+' || str->chars[i] == '-') {
        i = 1;
    }

    for (; i < str->length; i++) {
        if (!isdigit(str->chars[i])) {
            return false;
        }
    }

    return true;
}

static bool isBigInt(ObjAbstract* abstract) {
    return abstract->func == &freeBigInt;
}

static bool isBigIntValue(Value val) {
    if (!IS_ABSTRACT(val)) {
        return false;
    }

    return isBigInt((ObjAbstract*) AS_ABSTRACT(val));
}

static BigIntData* freeBigIntOnFailure(DictuVM *vm, BigIntData* bi, bigint* ptr) {
    if (bi != NULL && ptr == NULL) {
        FREE(vm, BigIntData, bi);
        return NULL;
    }

    return bi;
}

static BigIntData *allocBigInt(DictuVM *vm) {
    BigIntData *bi = ALLOCATE(vm, BigIntData, 1);
    bigint_init(&bi->value);
    return bi;
}

static BigIntData *cloneBigInt(DictuVM *vm, BigIntData* toClone) {
    BigIntData *bi = allocBigInt(vm);

    if (bi != NULL) {
        bigint_cpy(&bi->value, &toClone->value);
    }

    return bi;
}

static BigIntData *bigIntFromIntString(DictuVM *vm, Value strValue) {
    if (!isIntString(strValue)) {
        return NULL;
    }

    ObjString* str = AS_STRING(strValue);
    BigIntData *bi = allocBigInt(vm);
    bigint *ret = NULL;

    if (bi != NULL) {
        ret = bigint_from_str(&bi->value, str->chars);
    }

    return freeBigIntOnFailure(vm, bi, ret);
}

static BigIntData *bigIntFromLong(DictuVM *vm, Value numValue) {
    if (!isLong(numValue)) {
        return NULL;
    }

    long num = (long) AS_NUMBER(numValue);
    BigIntData *bi = allocBigInt(vm);
    bigint *ret = NULL;

    if (bi != NULL) {
        ret = bigint_from_int(&bi->value, num);
    }

    return freeBigIntOnFailure(vm, bi, ret);
}

static Value newBigIntValue(DictuVM *vm, BigIntData *bi) {
    if (bi == NULL) {
        return newResultError(vm, "error: failed to instantiate bigint");
    }

    ObjAbstract *abstract = newAbstract(vm, &freeBigInt, &bigIntToString);
    push(vm, OBJ_VAL(abstract));
    abstract->data = bi;

    /**
     * Setup BigInt object methods
     */
    defineNative(vm, &abstract->values, "compare",     &compare);
    defineNative(vm, &abstract->values, "negate",      &arithNeg);
    defineNative(vm, &abstract->values, "absoluteVal", &arithAbs);
    defineNative(vm, &abstract->values, "bitwiseAnd",  &bitwiseAnd);
    defineNative(vm, &abstract->values, "bitwiseOr",   &bitwiseOr);
    defineNative(vm, &abstract->values, "bitwiseXor",  &bitwiseXor);
    defineNative(vm, &abstract->values, "add",         &arithAdd);
    defineNative(vm, &abstract->values, "subtract",    &arithSub);
    defineNative(vm, &abstract->values, "multiply",    &arithMul);
    defineNative(vm, &abstract->values, "divide",      &arithDiv);
    defineNative(vm, &abstract->values, "modulo",      &arithMod);
    pop(vm);

    return newResultSuccess(vm, OBJ_VAL(abstract));
}

static Value newBigInt(DictuVM *vm, int argCount, Value *args) {
    if (argCount > 1) {
        return newResultError(vm, "error: wrong number of arguments");
    }

    if (argCount == 0) {
        return newBigIntValue(vm, bigIntFromLong(vm, 0));
    } else if (IS_NUMBER(args[0])) {
        return newBigIntValue(vm, bigIntFromLong(vm, args[0]));
    } else if (IS_STRING(args[0])) {
        return newBigIntValue(vm, bigIntFromIntString(vm, args[0]));
    } else if (isBigIntValue(args[0])) {
        return newBigIntValue(vm, cloneBigInt(vm, AS_ABSTRACT(args[0])->data));
    }

    return newResultError(vm, "error: invalid argument");
}

static Value applyOp1Arg(DictuVM *vm, int argCount, Value *args, int (*op)(const bigint*, const bigint*)) {
    if (argCount != 1) {
        return newResultError(vm, "error: wrong number of arguments");
    }

    if (!isBigIntValue(args[1])) {
        return newResultError(vm, "error: invalid argument");
    }

    int res = op(&AS_BIGINT(AS_ABSTRACT(args[0]))->value, &AS_BIGINT(AS_ABSTRACT(args[1]))->value);
    return NUMBER_VAL(res);
}

static Value dealWithReturn(DictuVM *vm,  BigIntData* bi, bigint* ptr) {
    bi = freeBigIntOnFailure(vm, bi, ptr);

    if (ptr == NULL) {
        return newResultError(vm, "error: operation error");
    } else {
        return newBigIntValue(vm, bi);
    }
}

static Value applyOp0ArgToReturn(DictuVM *vm, int argCount, Value *args, bigint* (*op)(bigint*)) {
    if (argCount != 0) {
        return newResultError(vm, "error: wrong number of arguments");
    }

    BigIntData *bi = cloneBigInt(vm, AS_ABSTRACT(args[0])->data);
    bigint *ret = NULL;

    if (bi != NULL) {
        ret = op(&bi->value);
    }

    return dealWithReturn(vm, bi, ret);
}

static Value applyOp1ArgAndReturn(DictuVM *vm, int argCount, Value *args, bigint* (*op)(bigint*, const bigint*, const bigint*)) {
    if (argCount != 1) {
        return newResultError(vm, "error: wrong number of arguments");
    }

    if (!isBigIntValue(args[1])) {
        return newResultError(vm, "error: invalid argument");
    }

    BigIntData *bi = allocBigInt(vm);
    bigint *ret = NULL;

    if (bi != NULL) {
        ret = op(&bi->value, &AS_BIGINT(AS_ABSTRACT(args[0]))->value, &AS_BIGINT(AS_ABSTRACT(args[1]))->value);
    }

    return dealWithReturn(vm, bi, ret);
}

APPLY_OP(compare,    Op1Arg,          &bigint_cmp)
APPLY_OP(arithNeg,   Op0ArgToReturn,  &bigint_negate)
APPLY_OP(arithAbs,   Op0ArgToReturn,  &bigint_abs)
APPLY_OP(bitwiseAnd, Op1ArgAndReturn, &bigint_and)
APPLY_OP(bitwiseOr,  Op1ArgAndReturn, &bigint_or)
APPLY_OP(bitwiseXor, Op1ArgAndReturn, &bigint_xor)
APPLY_OP(arithAdd,   Op1ArgAndReturn, &bigint_add)
APPLY_OP(arithSub,   Op1ArgAndReturn, &bigint_sub)
APPLY_OP(arithMul,   Op1ArgAndReturn, &bigint_mul)
APPLY_OP(arithDiv,   Op1ArgAndReturn, &bigint_div)
APPLY_OP(arithMod,   Op1ArgAndReturn, &bigint_mod)

Value createBigIntModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "BigInt", 6);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define BigInt methods
     */
    defineNative(vm, &module->values, "bigint", newBigInt);

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
