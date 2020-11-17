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

#define OK     0
#define NOTOK -1

// DictuVM *initVM(bool repl, int argc, char *argv[]);

// void dictuFreeVM(DictuVM *vm);

// InterpretResult interpret(DictuVM *vm, char *moduleName, char *source);

void push(DictuVM *vm, Value value);

Value peek(DictuVM *vm, int distance);

void runtimeError(DictuVM *vm, const char *format, ...);

Value pop(DictuVM *vm);

bool isFalsey(Value value);

#endif
