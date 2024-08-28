#ifndef dictu_ffi_include_h
#define dictu_ffi_include_h

#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// This is used ti determine if we can safely load the function pointers without
// UB.
#define FFI_MOD_API_VERSION 2

#define UNUSED(__x__) (void)__x__

#define MAX_ERROR_LEN 256

#define ERROR_RESULT                                                           \
    do {                                                                       \
        char buf[MAX_ERROR_LEN];                                               \
        getStrerror(buf, errno);                                               \
        return newResultError(vm, buf);                                        \
    } while (false)

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifdef _WIN32
#define DIR_SEPARATOR '\\'
#define DIR_ALT_SEPARATOR '/'
#define DIR_SEPARATOR_AS_STRING "\\"
#define DIR_SEPARATOR_STRLEN 1
#define PATH_DELIMITER ';'
#define PATH_DELIMITER_AS_STRING ";"
#define PATH_DELIMITER_STRLEN 1
#else
#define HAS_REALPATH
#define DIR_SEPARATOR '/'
#define DIR_SEPARATOR_AS_STRING "/"
#define DIR_SEPARATOR_STRLEN 1
#define PATH_DELIMITER ':'
#define PATH_DELIMITER_AS_STRING ":"
#define PATH_DELIMITER_STRLEN 1
#endif

#ifdef DIR_ALT_SEPARATOR
#define IS_DIR_SEPARATOR(c) ((c) == DIR_SEPARATOR || (c) == DIR_ALT_SEPARATOR)
#else
#define IS_DIR_SEPARATOR(c) (c == DIR_SEPARATOR)
#endif

#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION
#define DEBUG_TRACE_GC
#define DEBUG_TRACE_MEM

#ifndef _MSC_VER
#define COMPUTED_GOTO
#endif

#undef DEBUG_PRINT_CODE
#undef DEBUG_TRACE_EXECUTION
#undef DEBUG_TRACE_GC
#undef DEBUG_TRACE_MEM

// #define DEBUG_STRESS_GC
// #define DEBUG_FINAL_MEM

#define UINT8_COUNT (UINT8_MAX + 1)

typedef struct sObj Obj;
typedef struct sObjString ObjString;
typedef struct sObjList ObjList;
typedef struct sObjDict ObjDict;
typedef struct sObjSet ObjSet;
typedef struct sObjFile ObjFile;
typedef struct sObjAbstract ObjAbstract;
typedef struct sObjResult ObjResult;

// A mask that selects the sign bit.
#define SIGN_BIT ((uint64_t)1 << 63)

// The bits that must be set to indicate a quiet NaN.
#define QNAN ((uint64_t)0x7ffc000000000000)

// Tag values for the different singleton values.
#define TAG_NIL 1
#define TAG_FALSE 2
#define TAG_TRUE 3
#define TAG_EMPTY 4

typedef uint64_t Value;

#define IS_BOOL(v) (((v) | 1) == TRUE_VAL)
#define IS_NIL(v) ((v) == NIL_VAL)
#define IS_EMPTY(v) ((v) == EMPTY_VAL)
// If the NaN bits are set, it's not a number.
#define IS_NUMBER(v) (((v)&QNAN) != QNAN)
#define IS_OBJ(v) (((v) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

#define AS_BOOL(v) ((v) == TRUE_VAL)
#define AS_NUMBER(v) valueToNum(v)
#define AS_OBJ(v) ((Obj *)(uintptr_t)((v) & ~(SIGN_BIT | QNAN)))

#define BOOL_VAL(boolean) ((boolean) ? TRUE_VAL : FALSE_VAL)
#define FALSE_VAL ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL ((Value)(uint64_t)(QNAN | TAG_TRUE))
#define NIL_VAL ((Value)(uint64_t)(QNAN | TAG_NIL))
#define EMPTY_VAL ((Value)(uint64_t)(QNAN | TAG_EMPTY))
#define NUMBER_VAL(num) numToValue(num)
// The triple casting is necessary here to satisfy some compilers:
// 1. (uintptr_t) Convert the pointer to a number of the right size.
// 2. (uint64_t)  Pad it up to 64 bits in 32-bit builds.
// 3. Or in the bits to make a tagged Nan.
// 4. Cast to a typedef'd value.
#define OBJ_VAL(obj) (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))

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

typedef struct {
    ObjString *key;
    Value value;
    uint32_t psl;
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry *entries;
} Table;

typedef enum {
    OBJ_MODULE,
    OBJ_BOUND_METHOD,
    OBJ_CLASS,
    OBJ_ENUM,
    OBJ_CLOSURE,
    OBJ_FUNCTION,
    OBJ_INSTANCE,
    OBJ_NATIVE,
    OBJ_STRING,
    OBJ_LIST,
    OBJ_DICT,
    OBJ_SET,
    OBJ_FILE,
    OBJ_ABSTRACT,
    OBJ_RESULT,
    OBJ_UPVALUE
} ObjType;

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define AS_MODULE(value) ((ObjModule *)AS_OBJ(value))
#define AS_BOUND_METHOD(value) ((ObjBoundMethod *)AS_OBJ(value))
#define AS_CLASS(value) ((ObjClass *)AS_OBJ(value))
#define AS_ENUM(value) ((ObjEnum *)AS_OBJ(value))
#define AS_CLOSURE(value) ((ObjClosure *)AS_OBJ(value))
#define AS_FUNCTION(value) ((ObjFunction *)AS_OBJ(value))
#define AS_INSTANCE(value) ((ObjInstance *)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative *)AS_OBJ(value))->function)
#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)
#define AS_LIST(value) ((ObjList *)AS_OBJ(value))
#define AS_DICT(value) ((ObjDict *)AS_OBJ(value))
#define AS_SET(value) ((ObjSet *)AS_OBJ(value))
#define AS_FILE(value) ((ObjFile *)AS_OBJ(value))
#define AS_ABSTRACT(value) ((ObjAbstract *)AS_OBJ(value))
#define AS_RESULT(value) ((ObjResult *)AS_OBJ(value))

#define IS_MODULE(value) isObjType(value, OBJ_MODULE)
#define IS_BOUND_METHOD(value) isObjType(value, OBJ_BOUND_METHOD)
#define IS_CLASS(value) isObjType(value, OBJ_CLASS)
#define IS_DEFAULT_CLASS(value)                                                \
    isObjType(value, OBJ_CLASS) && AS_CLASS(value)->type == CLASS_DEFAULT
#define IS_TRAIT(value)                                                        \
    isObjType(value, OBJ_CLASS) && AS_CLASS(value)->type == CLASS_TRAIT
#define IS_ENUM(value) isObjType(value, OBJ_ENUM)
#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_INSTANCE(value) isObjType(value, OBJ_INSTANCE)
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)
#define IS_STRING(value) isObjType(value, OBJ_STRING)
#define IS_LIST(value) isObjType(value, OBJ_LIST)
#define IS_DICT(value) isObjType(value, OBJ_DICT)
#define IS_SET(value) isObjType(value, OBJ_SET)
#define IS_FILE(value) isObjType(value, OBJ_FILE)
#define IS_ABSTRACT(value) isObjType(value, OBJ_ABSTRACT)
#define IS_RESULT(value) isObjType(value, OBJ_RESULT)



typedef enum { CLASS_DEFAULT, CLASS_ABSTRACT, CLASS_TRAIT } ClassType;

typedef enum { ACCESS_PUBLIC, ACCESS_PRIVATE } AccessLevel;

typedef enum {
    TYPE_FUNCTION,
    TYPE_ARROW_FUNCTION,
    TYPE_INITIALIZER,
    TYPE_METHOD,
    TYPE_STATIC,
    TYPE_ABSTRACT,
    TYPE_TOP_LEVEL
} FunctionType;

struct sObj {
    ObjType type;
    bool isDark;
    struct sObj *next;
};
static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

typedef struct sUpvalue {
    Obj obj;

    // Pointer to the variable this upvalue is referencing.
    Value *value;

    // If the upvalue is closed (i.e. the local variable it was pointing
    // to has been popped off the stack) then the closed-over value is
    // hoisted out of the stack into here. [value] is then be changed to
    // point to this.
    Value closed;

    // Open upvalues are stored in a linked list. This points to the next
    // one in that list.
    struct sUpvalue *next;
} ObjUpvalue;

typedef struct {
    Obj obj;
    ObjString *name;
    ObjString *path;
    Table values;
} ObjModule;

typedef struct {
    int count;
    int capacity;
    uint8_t *code;
    int *lines;
    ValueArray constants;
} Chunk;

typedef struct {
    Obj obj;
    int isVariadic;
    int arity;
    int arityOptional;
    int upvalueCount;
    Chunk chunk;
    ObjString *name;
    FunctionType type;
    AccessLevel accessLevel;
    ObjModule *module;
    int propertyCount;
    int *propertyNames;
    int *propertyIndexes;
    int privatePropertyCount;
    int *privatePropertyNames;
    int *privatePropertyIndexes;
} ObjFunction;

#define STACK_MAX (64 * UINT8_COUNT)
typedef struct {
    Obj obj;
    ObjFunction *function;
    ObjUpvalue **upvalues;
    int upvalueCount;
} ObjClosure;

typedef struct {
    ObjClosure *closure;
    uint8_t *ip;
    Value *slots;
} CallFrame;

struct _vm {
    void* _compilerStub;
    Value stack[STACK_MAX];
    Value *stackTop;
    bool repl;
    CallFrame *frames;
    int frameCount;
    int frameCapacity;
    ObjModule *lastModule;
    Table modules;
    Table globals;
    Table constants;
    Table strings;
    Table numberMethods;
    Table boolMethods;
    Table nilMethods;
    Table stringMethods;
    Table listMethods;
    Table dictMethods;
    Table setMethods;
    Table fileMethods;
    Table classMethods;
    Table instanceMethods;
    Table resultMethods;
    Table enumMethods;
    ObjString *initString;
    ObjString *annotationString;
    ObjString *replVar;
    ObjUpvalue *openUpvalues;
    size_t bytesAllocated;
    size_t nextGC;
    Obj *objects;
    int grayCount;
    int grayCapacity;
    Obj **grayStack;
    int argc;
    char **argv;
};

#define DICTU_MAJOR_VERSION "0"
#define DICTU_MINOR_VERSION "29"
#define DICTU_PATCH_VERSION "0"

#define DICTU_STRING_VERSION                                                   \
    "Dictu Version: " DICTU_MAJOR_VERSION "." DICTU_MINOR_VERSION              \
    "." DICTU_PATCH_VERSION "\n"

typedef struct _vm DictuVM;

typedef Value (*NativeFn)(DictuVM *vm, int argCount, Value *args);

typedef struct {
    Obj obj;
    NativeFn function;
} ObjNative;

struct sObjString {
    Obj obj;
    int length;
    char *chars;
    uint32_t hash;
};

struct sObjList {
    Obj obj;
    ValueArray values;
};

typedef struct {
    Value key;
    Value value;
} DictItem;

struct sObjDict {
    Obj obj;
    int count;
    int activeCount;
    int capacityMask;
    DictItem *entries;
};

typedef struct {
    Value value;
    bool deleted;
} SetItem;

struct sObjSet {
    Obj obj;
    int count;
    int capacityMask;
    SetItem *entries;
};

struct sObjFile {
    Obj obj;
    FILE *file;
    char *path;
    char *openType;
};

typedef void (*AbstractFreeFn)(DictuVM *vm, ObjAbstract *abstract);
typedef void (*AbstractGrayFn)(DictuVM *vm, ObjAbstract *abstract);
typedef char *(*AbstractTypeFn)(ObjAbstract *abstract);

struct sObjAbstract {
    Obj obj;
    Table values;
    void *data;
    AbstractFreeFn func;
    AbstractGrayFn grayFunc;
    AbstractTypeFn type;
    bool excludeSelf;
};

typedef enum { SUCCESS, ERR } ResultStatus;

struct sObjResult {
    Obj obj;
    ResultStatus status;
    Value value;
};

typedef struct sObjClass {
    Obj obj;
    ObjString *name;
    struct sObjClass *superclass;
    Table publicMethods;
    Table privateMethods;
    Table abstractMethods;
    Table variables;
    Table constants;
    ObjDict *classAnnotations;
    ObjDict *methodAnnotations;
    ObjDict *fieldAnnotations;
    ClassType type;
} ObjClass;

typedef struct sObjEnum {
    Obj obj;
    ObjString *name;
    Table values;
} ObjEnum;

typedef struct {
    Obj obj;
    ObjClass *klass;
    Table privateAttributes;
    Table publicAttributes;
} ObjInstance;

typedef struct {
    Obj obj;
    Value receiver;
    ObjClosure *method;
} ObjBoundMethod;

#define ALLOCATE(vm, type, count) \
    (type*)reallocate(vm, NULL, 0, sizeof(type) * (count))

#define FREE(vm, type, pointer) \
    reallocate(vm, pointer, sizeof(type), 0)

#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define SHRINK_CAPACITY(capacity) \
    ((capacity) < 16 ? 8 : (capacity) / 2)

#define GROW_ARRAY(vm, previous, type, oldCount, count) \
    (type*)reallocate(vm, previous, sizeof(type) * (oldCount), \
        sizeof(type) * (count))

#define SHRINK_ARRAY(vm, previous, type, oldCount, count) \
    (type*)reallocate(vm, previous, sizeof(type) * (oldCount), \
        sizeof(type) * (count))

#define FREE_ARRAY(vm, type, pointer, oldCount) \
    reallocate(vm, pointer, sizeof(type) * (oldCount), 0)

typedef void *reallocate_t(DictuVM *vm, void *previous, size_t oldSize, size_t newSize);

typedef ObjString *copyString_t(DictuVM *vm, const char *chars, int length);

typedef ObjList *newList_t(DictuVM *vm);

typedef ObjDict *newDict_t(DictuVM *vm);

typedef ObjSet *newSet_t(DictuVM *vm);

typedef ObjFile *newFile_t(DictuVM *vm);

typedef ObjAbstract *newAbstract_t(DictuVM *vm, AbstractFreeFn func,
                                   AbstractTypeFn type);

typedef ObjResult *newResult_t(DictuVM *vm, ResultStatus status, Value value);

typedef Value newResultSuccess_t(DictuVM *vm, Value value);

typedef Value newResultError_t(DictuVM *vm, char *errorMsg);

typedef void push_t(DictuVM *vm, Value value);

typedef Value peek_t(DictuVM *vm, int distance);

typedef void runtimeError_t(DictuVM *vm, const char *format, ...);

typedef Value pop_t(DictuVM *vm);

typedef bool isFalsey_t(Value value);

typedef bool valuesEqual_t(Value a, Value b);

typedef void initValueArray_t(ValueArray *array);

typedef void writeValueArray_t(DictuVM *vm, ValueArray *array, Value value);

typedef void freeValueArray_t(DictuVM *vm, ValueArray *array);

typedef bool dictSet_t(DictuVM *vm, ObjDict *dict, Value key, Value value);

typedef bool dictGet_t(ObjDict *dict, Value key, Value *value);

typedef bool dictDelete_t(DictuVM *vm, ObjDict *dict, Value key);

typedef bool setGet_t(ObjSet *set, Value value);

typedef bool setInsert_t(DictuVM *vm, ObjSet *set, Value value);

typedef bool setDelete_t(DictuVM *vm, ObjSet *set, Value value);

typedef char *valueToString_t(Value value);

typedef char *valueTypeToString_t(DictuVM *vm, Value value, int *length);

typedef void printValue_t(Value value);

typedef void printValueError_t(Value value);

typedef bool compareStringLess_t(Value a, Value b);

typedef bool compareStringGreater_t(Value a, Value b);

typedef void defineNative_t(DictuVM *vm, Table *table, const char *name,
                            NativeFn function);

typedef void defineNativeProperty_t(DictuVM *vm, Table *table, const char *name,
                                    Value value);
reallocate_t * reallocate = NULL;

copyString_t *copyString = NULL;

newList_t *newList = NULL;

newDict_t *newDict = NULL;

newSet_t *newSet = NULL;

newFile_t *newFile = NULL;

newAbstract_t *newAbstract = NULL;

newResult_t *newResult = NULL;

newResultSuccess_t *newResultSuccess = NULL;

newResultError_t *newResultError = NULL;

push_t *push = NULL;

peek_t *peek = NULL;

runtimeError_t *runtimeError = NULL;

pop_t *pop = NULL;

isFalsey_t *isFalsey = NULL;

valuesEqual_t *valuesEqual = NULL;

initValueArray_t *initValueArray = NULL;

writeValueArray_t *writeValueArray = NULL;

freeValueArray_t *freeValueArray = NULL;

dictSet_t *dictSet = NULL;

dictGet_t *dictGet = NULL;

dictDelete_t *dictDelete = NULL;

setGet_t *setGet = NULL;

setInsert_t *setInsert = NULL;

setDelete_t *setDelete = NULL;

valueToString_t *valueToString = NULL;

valueTypeToString_t *valueTypeToString = NULL;

printValue_t *printValue = NULL;

printValueError_t *printValueError = NULL;

compareStringLess_t *compareStringLess = NULL;

compareStringGreater_t *compareStringGreater = NULL;

defineNative_t *defineNative = NULL;

defineNativeProperty_t *defineNativeProperty = NULL;

// This needs to be implemented by the user and register all functions
int dictu_ffi_init(DictuVM *vm, Table *method_table);

#ifdef _WIN32
__declspec(dllexport)
#endif
int dictu_internal_ffi_init(void **function_ptrs, DictuVM *vm,
                            Table *methodTable, int vm_ffi_version) {
    if (copyString != NULL) {
        // we already initialized.
        return 1;
    }
    if (FFI_MOD_API_VERSION > vm_ffi_version)
        return 2;
    size_t count = 0;

    copyString = (copyString_t *)function_ptrs[count++];
    newList = (newList_t *)function_ptrs[count++];
    newDict = (newDict_t *)function_ptrs[count++];
    newSet = (newSet_t *)function_ptrs[count++];
    newFile = (newFile_t *)function_ptrs[count++];
    newAbstract = (newAbstract_t *)function_ptrs[count++];
    newResult = (newResult_t *)function_ptrs[count++];
    newResultSuccess = (newResultSuccess_t *)function_ptrs[count++];
    newResultError = (newResultError_t *)function_ptrs[count++];
    push = (push_t *)function_ptrs[count++];
    peek = (peek_t *)function_ptrs[count++];
    runtimeError = (runtimeError_t *)function_ptrs[count++];
    pop = (pop_t *)function_ptrs[count++];
    isFalsey = (isFalsey_t *)function_ptrs[count++];
    valuesEqual = (valuesEqual_t *)function_ptrs[count++];
    initValueArray = (initValueArray_t *)function_ptrs[count++];
    writeValueArray = (writeValueArray_t *)function_ptrs[count++];
    freeValueArray = (freeValueArray_t *)function_ptrs[count++];
    dictSet = (dictSet_t *)function_ptrs[count++];
    dictGet = (dictGet_t *)function_ptrs[count++];
    dictDelete = (dictDelete_t *)function_ptrs[count++];
    setGet = (setGet_t *)function_ptrs[count++];
    setInsert = (setInsert_t *)function_ptrs[count++];
    setDelete = (setDelete_t *)function_ptrs[count++];
    valueToString = (valueToString_t *)function_ptrs[count++];
    valueTypeToString = (valueTypeToString_t *)function_ptrs[count++];
    printValue = (printValue_t *)function_ptrs[count++];
    printValueError = (printValueError_t *)function_ptrs[count++];
    compareStringLess = (compareStringLess_t *)function_ptrs[count++];
    compareStringGreater = (compareStringGreater_t *)function_ptrs[count++];
    defineNative = (defineNative_t *)function_ptrs[count++];
    defineNativeProperty = (defineNativeProperty_t *)function_ptrs[count++];
    reallocate = (reallocate_t *)function_ptrs[count++];
    int initResult = dictu_ffi_init(vm, methodTable);
    if (initResult > 0)
        return 3 + initResult;
    return 0;
}
#ifdef __cplusplus
}
#endif
#endif
