#ifndef dictu_vm_h
#define dictu_vm_h

#include "object.h"
#include "table.h"
#include "value.h"
// TODO: Work out the maximum stack size at compilation time
#define STACK_MAX (64 * UINT8_COUNT)

typedef struct {
    ObjClosure *closure;
    uint8_t *ip;
    Value *slots;
} CallFrame;

typedef struct {
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
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

void initVM(bool repl, const char *scriptName, int argc, const char *argv[]);

void freeVM();

InterpretResult interpret(const char *source);

void push(Value value);

Value peek(int distance);

void runtimeError(const char *format, ...);

Value pop();

bool isFalsey(Value value);

#endif
