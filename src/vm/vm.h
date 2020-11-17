#ifndef dictu_vm_h
#define dictu_vm_h

#include "object.h"
#include "table.h"
#include "value.h"
#include "compiler.h"

// TODO: Work out the maximum stack size at compilation time
#define STACK_MAX (64 * UINT8_COUNT)

typedef struct {
    ObjClosure *closure;
    uint8_t *ip;
    Value *slots;
} CallFrame;

struct _vm {
    Compiler *compiler;
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
    Table socketMethods;
    ObjString *initString;
    ObjString *replVar;
    ObjUpvalue *openUpvalues;
    size_t bytesAllocated;
    size_t nextGC;
    Obj *objects;
    int grayCount;
    int grayCapacity;
    Obj **grayStack;
};

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

// extern VM vm;

#define OK     0
#define NOTOK -1

VM *initVM(bool repl, int argc, char *argv[]);

void freeVM(VM *vm);

InterpretResult interpret(VM *vm, char *moduleName, char *source);

void push(VM *vm, Value value);

Value peek(VM *vm, int distance);

void runtimeError(VM *vm, const char *format, ...);

Value pop(VM *vm);

bool isFalsey(Value value);

#endif
