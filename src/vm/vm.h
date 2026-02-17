#ifndef dictu_vm_h
#define dictu_vm_h

#include "object.h"
#include "table.h"
#include "value.h"
#include "compiler.h"

#define STACK_INITIAL (64 * UINT8_COUNT)

struct _vm {
    Compiler *compiler;
    ObjFiber *fiber;
    bool fiberSwitch;
    bool repl;
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
    Table fiberMethods;
    ObjString *initString;
    ObjString *annotationString;
    ObjString *replVar;
    size_t bytesAllocated;
    size_t nextGC;
    Obj *objects;
    int grayCount;
    int grayCapacity;
    Obj **grayStack;
    int argc;
    char **argv;
};

#define OK     0
#define NOTOK -1

void push(DictuVM *vm, Value value);

Value peek(DictuVM *vm, int distance);

void runtimeError(DictuVM *vm, const char *format, ...);

Value pop(DictuVM *vm);

bool isFalsey(Value value);

ObjClosure *compileModuleToClosure(DictuVM *vm, char *name, char *source);

Value callFunction(DictuVM* vm, Value function, int argCount, Value* args);

#endif
