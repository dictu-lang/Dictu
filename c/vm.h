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
    int stackCount;
    bool repl;
    bool gc;
    const char *scriptName;
    const char *currentScriptName;
    CallFrame *frames;
    int frameCount;
    int frameCapacity;
    int currentFrameCount;
    Table globals;
    Table strings;
    Table imports;
    ObjString *initString;
    ObjString *replVar;
    ObjString *argv;
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

VM *initVM(bool repl, const char *scriptName, int argc, const char *argv[]);

void freeVM(VM *vm);

InterpretResult interpret(VM *vm, const char *source);

void push(VM *vm, Value value);

Value peek(VM *vm, int distance);

void runtimeError(VM *vm, const char *format, ...);

Value pop(VM *vm);

bool isFalsey(Value value);

#endif
