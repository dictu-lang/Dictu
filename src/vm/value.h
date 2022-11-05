#ifndef dictu_value_h
#define dictu_value_h

#include "../include/dictu_include.h"
#include "common.h"

typedef struct sObj Obj;
typedef struct sObjString ObjString;
typedef struct sObjList ObjList;
typedef struct sObjDict ObjDict;
typedef struct sObjSet  ObjSet;
typedef struct sObjFile ObjFile;
typedef struct sObjAbstract ObjAbstract;
typedef struct sObjResult ObjResult;

// A mask that selects the sign bit.
#define SIGN_BIT ((uint64_t)1 << 63)

// The bits that must be set to indicate a quiet NaN.
#define QNAN ((uint64_t)0x7ffc000000000000)

// Tag values for the different singleton values.
#define TAG_NIL    1
#define TAG_FALSE  2
#define TAG_TRUE   3
#define TAG_EMPTY  4

typedef uint64_t Value;

#define IS_BOOL(v)    (((v) | 1) == TRUE_VAL)
#define IS_NIL(v)     ((v) == NIL_VAL)
#define IS_EMPTY(v)   ((v) == EMPTY_VAL)
// If the NaN bits are set, it's not a number.
#define IS_NUMBER(v)  (((v) & QNAN) != QNAN)
#define IS_OBJ(v)     (((v) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

#define AS_BOOL(v)    ((v) == TRUE_VAL)
#define AS_NUMBER(v)  valueToNum(v)
#define AS_OBJ(v)     ((Obj*)(uintptr_t)((v) & ~(SIGN_BIT | QNAN)))

#define BOOL_VAL(boolean)   ((boolean) ? TRUE_VAL : FALSE_VAL)
#define FALSE_VAL           ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL            ((Value)(uint64_t)(QNAN | TAG_TRUE))
#define NIL_VAL             ((Value)(uint64_t)(QNAN | TAG_NIL))
#define EMPTY_VAL           ((Value)(uint64_t)(QNAN | TAG_EMPTY))
#define NUMBER_VAL(num)   numToValue(num)
// The triple casting is necessary here to satisfy some compilers:
// 1. (uintptr_t) Convert the pointer to a number of the right size.
// 2. (uint64_t)  Pad it up to 64 bits in 32-bit builds.
// 3. Or in the bits to make a tagged Nan.
// 4. Cast to a typedef'd value.
#define OBJ_VAL(obj) \
    (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))

// A union to let us reinterpret a double as raw bits and back.
typedef union {
    uint64_t bits64;
    uint32_t bits32[2];
    double num;
} DoubleUnion;

static inline double valueToNum(Value value) {
    DoubleUnion data;
    data.bits64 = value;
    return data.num;
}

static inline Value numToValue(double num) {
    DoubleUnion data;
    data.num = num;
    return data.bits64;
}

typedef struct {
    int capacity;
    int count;
    Value *values;
} ValueArray;

bool valuesEqual(Value a, Value b);

void initValueArray(ValueArray *array);

void writeValueArray(DictuVM *vm, ValueArray *array, Value value);

void freeValueArray(DictuVM *vm, ValueArray *array);

void grayDict(DictuVM *vm, ObjDict *dict);

bool dictSet(DictuVM *vm, ObjDict *dict, Value key, Value value);

bool dictGet(ObjDict *dict, Value key, Value *value);

bool dictDelete(DictuVM *vm, ObjDict *dict, Value key);

bool setGet(ObjSet *set, Value value);

bool setInsert(DictuVM *vm, ObjSet *set, Value value);

bool setDelete(DictuVM *vm, ObjSet *set, Value value);

void graySet(DictuVM *vm, ObjSet *set);

char *valueToString(DictuVM *vm, Value value);

char *valueTypeToString(DictuVM *vm, Value value, int *length);

void printValue(DictuVM *vm, Value value);

void printValueError(DictuVM *vm, Value value);

#endif
