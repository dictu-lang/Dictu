#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
#include "vm.h"
#include "util.h"
#include "datatypes/number.h"
#include "datatypes/bool.h"
#include "datatypes/nil.h"
#include "datatypes/strings.h"
#include "datatypes/lists.h"
#include "datatypes/dicts.h"
#include "datatypes/sets.h"
#include "datatypes/files.h"
#include "datatypes/class.h"
#include "datatypes/instance.h"
#include "natives.h"
#include "optionals/optionals.h"

static void resetStack(VM *vm) {
    vm->stackTop = vm->stack;
    vm->frameCount = 0;
    vm->openUpvalues = NULL;
    vm->compiler = NULL;
}

void runtimeError(VM *vm, const char *format, ...) {
    for (int i = vm->frameCount - 1; i >= 0; i--) {
        CallFrame *frame = &vm->frames[i];

        ObjFunction *function = frame->closure->function;

        // -1 because the IP is sitting on the next instruction to be
        // executed.
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[line %d] in ",
                function->chunk.lines[instruction]);

        if (function->name == NULL) {
            fprintf(stderr, "%s: ", vm->scriptNames[vm->scriptNameCount]);
            i = -1;
        } else {
            fprintf(stderr, "%s(): ", function->name->chars);
        }

        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        fputs("\n", stderr);
        va_end(args);
    }

    resetStack(vm);
}

void setupFilenameStack(VM *vm, const char *scriptName) {
    vm->scriptNameCapacity = 8;
    vm->scriptNames = ALLOCATE(vm, const char*, vm->scriptNameCapacity);
    vm->scriptNameCount = 0;
    vm->scriptNames[vm->scriptNameCount] = scriptName;
}

void setcurrentFile(VM *vm, const char *scriptname, int len) {
    ObjString *name = copyString(vm, scriptname, len);
    push(vm, OBJ_VAL(name));
    ObjString *__file__ = copyString(vm, "__file__", 8);
    push(vm, OBJ_VAL(__file__));
    tableSet(vm, &vm->globals, __file__, OBJ_VAL(name));
    pop(vm);
    pop(vm);
}

VM *initVM(bool repl, const char *scriptName, int argc, const char *argv[]) {
    VM *vm = malloc(sizeof(*vm));

    if (vm == NULL) {
        printf("Unable to allocate memory\n");
        exit(71);
    }

    memset(vm, '\0', sizeof(VM));

    resetStack(vm);
    vm->objects = NULL;
    vm->repl = repl;
    vm->frameCapacity = 4;
    vm->frames = NULL;
    vm->initString = NULL;
    vm->replVar = NULL;
    vm->bytesAllocated = 0;
    vm->nextGC = 1024 * 1024;
    vm->grayCount = 0;
    vm->grayCapacity = 0;
    vm->grayStack = NULL;
    initTable(&vm->globals);
    initTable(&vm->constants);
    initTable(&vm->strings);
    initTable(&vm->imports);

    initTable(&vm->numberMethods);
    initTable(&vm->boolMethods);
    initTable(&vm->nilMethods);
    initTable(&vm->stringMethods);
    initTable(&vm->listMethods);
    initTable(&vm->dictMethods);
    initTable(&vm->setMethods);
    initTable(&vm->fileMethods);
    initTable(&vm->classMethods);
    initTable(&vm->instanceMethods);

    setupFilenameStack(vm, scriptName);
    if (scriptName == NULL) {
        setcurrentFile(vm, "", 0);
    } else {
        setcurrentFile(vm, scriptName, (int) strlen(scriptName));
    }

    vm->frames = ALLOCATE(vm, CallFrame, vm->frameCapacity);
    vm->initString = copyString(vm, "init", 4);
    vm->replVar = copyString(vm, "_", 1);

    // Native methods
    declareNumberMethods(vm);
    declareBoolMethods(vm);
    declareNilMethods(vm);
    declareStringMethods(vm);
    declareListMethods(vm);
    declareDictMethods(vm);
    declareSetMethods(vm);
    declareFileMethods(vm);
    declareClassMethods(vm);
    declareInstanceMethods(vm);

    // Native functions
    defineAllNatives(vm);

    // Native classes
    createMathsClass(vm);
    createEnvClass(vm);
    createSystemClass(vm, argc, argv);
    createJSONClass(vm);
    createPathClass(vm);
#ifndef DISABLE_HTTP
    createHTTPClass(vm);
#endif
    return vm;
}

void freeVM(VM *vm) {
    freeTable(vm, &vm->globals);
    freeTable(vm, &vm->constants);
    freeTable(vm, &vm->strings);
    freeTable(vm, &vm->imports);
    freeTable(vm, &vm->numberMethods);
    freeTable(vm, &vm->boolMethods);
    freeTable(vm, &vm->nilMethods);
    freeTable(vm, &vm->stringMethods);
    freeTable(vm, &vm->listMethods);
    freeTable(vm, &vm->dictMethods);
    freeTable(vm, &vm->setMethods);
    freeTable(vm, &vm->fileMethods);
    freeTable(vm, &vm->classMethods);
    freeTable(vm, &vm->instanceMethods);
    FREE_ARRAY(vm, CallFrame, vm->frames, vm->frameCapacity);
    FREE_ARRAY(vm, const char*, vm->scriptNames, vm->scriptNameCapacity);
    vm->initString = NULL;
    vm->replVar = NULL;
    freeObjects(vm);

#if defined(DEBUG_TRACE_MEM) || defined(DEBUG_FINAL_MEM)
    printf("Total memory usage: %zu\n", vm->bytesAllocated);
#endif

    free(vm);
}

void push(VM *vm, Value value) {
    *vm->stackTop = value;
    vm->stackTop++;
}

Value pop(VM *vm) {
    vm->stackTop--;
    return *vm->stackTop;
}

Value peek(VM *vm, int distance) {
    return vm->stackTop[-1 - distance];
}

static bool call(VM *vm, ObjClosure *closure, int argCount) {
    if (argCount < closure->function->arity || argCount > closure->function->arity + closure->function->arityOptional) {
        runtimeError(vm, "Function '%s' expected %d arguments but got %d.",
                     closure->function->name->chars,
                     closure->function->arity + closure->function->arityOptional,
                     argCount
        );

        return false;
    }
    if (vm->frameCount == vm->frameCapacity) {
        int oldCapacity = vm->frameCapacity;
        vm->frameCapacity = GROW_CAPACITY(vm->frameCapacity);
        vm->frames = GROW_ARRAY(vm, vm->frames, CallFrame,
                                   oldCapacity, vm->frameCapacity);
    }

    CallFrame *frame = &vm->frames[vm->frameCount++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;

    frame->slots = vm->stackTop - argCount - 1;

    return true;
}

static bool callValue(VM *vm, Value callee, int argCount) {
    if (IS_OBJ(callee)) {
        switch (OBJ_TYPE(callee)) {
            case OBJ_BOUND_METHOD: {
                ObjBoundMethod *bound = AS_BOUND_METHOD(callee);

                // Replace the bound method with the receiver so it's in the
                // right slot when the method is called.
                vm->stackTop[-argCount - 1] = bound->receiver;
                return call(vm, bound->method, argCount);
            }

            case OBJ_CLASS: {
                ObjClass *klass = AS_CLASS(callee);

                // Create the instance.
                vm->stackTop[-argCount - 1] = OBJ_VAL(newInstance(vm, klass));

                // Call the initializer, if there is one.
                Value initializer;
                if (tableGet(&klass->methods, vm->initString, &initializer)) {
                    return call(vm, AS_CLOSURE(initializer), argCount);
                } else if (argCount != 0) {
                    runtimeError(vm, "Expected 0 arguments but got %d.", argCount);
                    return false;
                }

                return true;
            }

            case OBJ_CLOSURE:
                return call(vm, AS_CLOSURE(callee), argCount);

            case OBJ_NATIVE: {
                NativeFn native = AS_NATIVE(callee);
                Value result = native(vm, argCount, vm->stackTop - argCount);

                if (IS_EMPTY(result))
                    return false;

                vm->stackTop -= argCount + 1;
                push(vm, result);
                return true;
            }

            default:
                // Do nothing.
                break;
        }
    }

    runtimeError(vm, "Can only call functions and classes.");
    return false;
}

static bool callNativeMethod(VM *vm, Value method, int argCount) {
    NativeFn native = AS_NATIVE(method);

    Value result = native(vm, argCount, vm->stackTop - argCount - 1);

    if (IS_EMPTY(result))
        return false;

    vm->stackTop -= argCount + 1;
    push(vm, result);
    return true;
}

static bool invokeFromClass(VM *vm, ObjClass *klass, ObjString *name,
                            int argCount) {
    // Look for the method.
    Value method;
    if (!tableGet(&klass->methods, name, &method)) {
        runtimeError(vm, "Undefined property '%s'.", name->chars);
        return false;
    }

    return call(vm, AS_CLOSURE(method), argCount);
}

static bool invoke(VM *vm, ObjString *name, int argCount) {
    Value receiver = peek(vm, argCount);

    if (!IS_OBJ(receiver)) {
        if (IS_NUMBER(receiver)) {
            Value value;
            if (tableGet(&vm->numberMethods, name, &value)) {
                return callNativeMethod(vm, value, argCount);
            }

            runtimeError(vm, "Number has no method %s().", name->chars);
            return false;
        } else if (IS_BOOL(receiver)) {
            Value value;
            if (tableGet(&vm->boolMethods, name, &value)) {
                return callNativeMethod(vm, value, argCount);
            }

            runtimeError(vm, "Bool has no method %s().", name->chars);
            return false;
        } else if (IS_NIL(receiver)) {
            Value value;
            if (tableGet(&vm->nilMethods, name, &value)) {
                return callNativeMethod(vm, value, argCount);
            }

            runtimeError(vm, "Nil has no method %s().", name->chars);
            return false;
        }
    } else {
        switch (getObjType(receiver)) {
            case OBJ_NATIVE_CLASS: {
                ObjClassNative *instance = AS_CLASS_NATIVE(receiver);
                Value function;
                if (!tableGet(&instance->methods, name, &function)) {
                    runtimeError(vm, "Undefined property '%s'.", name->chars);
                    return false;
                }

                return callValue(vm, function, argCount);
            }

            case OBJ_CLASS: {
                ObjClass *instance = AS_CLASS(receiver);
                Value method;
                if (tableGet(&instance->methods, name, &method)) {
                    if (!AS_CLOSURE(method)->function->staticMethod) {
                        if (tableGet(&vm->classMethods, name, &method)) {
                            return callNativeMethod(vm, method, argCount);
                        }

                        runtimeError(vm, "'%s' is not static. Only static methods can be invoked directly from a class.",
                                     name->chars);
                        return false;
                    }

                    return callValue(vm, method, argCount);
                }

                if (tableGet(&vm->classMethods, name, &method)) {
                    return callNativeMethod(vm, method, argCount);
                }

                runtimeError(vm, "Undefined property '%s'.", name->chars);
                return false;
            }

            case OBJ_INSTANCE: {
                ObjInstance *instance = AS_INSTANCE(receiver);

                Value value;
                // First look for a field which may shadow a method.
                if (tableGet(&instance->fields, name, &value)) {
                    vm->stackTop[-argCount - 1] = value;
                    return callValue(vm, value, argCount);
                }

                // Look for the method.
                if (tableGet(&instance->klass->methods, name, &value)) {
                    return call(vm, AS_CLOSURE(value), argCount);
                }

                // Check for instance methods.
                if (tableGet(&vm->instanceMethods, name, &value)) {
                    return callNativeMethod(vm, value, argCount);
                }

                runtimeError(vm, "Undefined property '%s'.", name->chars);
                return false;
            }

            case OBJ_STRING: {
                Value value;
                if (tableGet(&vm->stringMethods, name, &value)) {
                    return callNativeMethod(vm, value, argCount);
                }

                runtimeError(vm, "String has no method %s().", name->chars);
                return false;
            }

            case OBJ_LIST: {
                Value value;
                if (tableGet(&vm->listMethods, name, &value)) {
                    return callNativeMethod(vm, value, argCount);
                }

                runtimeError(vm, "List has no method %s().", name->chars);
                return false;
            }

            case OBJ_DICT: {
                Value value;
                if (tableGet(&vm->dictMethods, name, &value)) {
                    return callNativeMethod(vm, value, argCount);
                }

                runtimeError(vm, "Dict has no method %s().", name->chars);
                return false;
            }

            case OBJ_SET: {
                Value value;
                if (tableGet(&vm->setMethods, name, &value)) {
                    return callNativeMethod(vm, value, argCount);
                }

                runtimeError(vm, "Set has no method %s().", name->chars);
                return false;
            }

            case OBJ_FILE: {
                Value value;
                if (tableGet(&vm->fileMethods, name, &value)) {
                    return callNativeMethod(vm, value, argCount);
                }

                runtimeError(vm, "File has no method %s().", name->chars);
                return false;
            }

            default:
                break;
        }
    }

    runtimeError(vm, "Only instances have methods.");
    return false;
}

static bool bindMethod(VM *vm, ObjClass *klass, ObjString *name) {
    Value method;
    if (!tableGet(&klass->methods, name, &method)) {
        runtimeError(vm, "Undefined property '%s'.", name->chars);
        return false;
    }

    ObjBoundMethod *bound = newBoundMethod(vm, peek(vm, 0), AS_CLOSURE(method));
    pop(vm); // Instance.
    push(vm, OBJ_VAL(bound));
    return true;
}

// Captures the local variable [local] into an [Upvalue]. If that local
// is already in an upvalue, the existing one is used. (This is
// important to ensure that multiple closures closing over the same
// variable actually see the same variable.) Otherwise, it creates a
// new open upvalue and adds it to the VM's list of upvalues.
static ObjUpvalue *captureUpvalue(VM *vm, Value *local) {
    // If there are no open upvalues at all, we must need a new one.
    if (vm->openUpvalues == NULL) {
        vm->openUpvalues = newUpvalue(vm, local);
        return vm->openUpvalues;
    }

    ObjUpvalue *prevUpvalue = NULL;
    ObjUpvalue *upvalue = vm->openUpvalues;

    // Walk towards the bottom of the stack until we find a previously
    // existing upvalue or reach where it should be.
    while (upvalue != NULL && upvalue->value > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    // If we found it, reuse it.
    if (upvalue != NULL && upvalue->value == local) return upvalue;

    // We walked past the local on the stack, so there must not be an
    // upvalue for it already. Make a new one and link it in in the right
    // place to keep the list sorted.
    ObjUpvalue *createdUpvalue = newUpvalue(vm, local);
    createdUpvalue->next = upvalue;

    if (prevUpvalue == NULL) {
        // The new one is the first one in the list.
        vm->openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->next = createdUpvalue;
    }

    return createdUpvalue;
}

static void closeUpvalues(VM *vm, Value *last) {
    while (vm->openUpvalues != NULL &&
           vm->openUpvalues->value >= last) {
        ObjUpvalue *upvalue = vm->openUpvalues;

        // Move the value into the upvalue itself and point the upvalue to
        // it.
        upvalue->closed = *upvalue->value;
        upvalue->value = &upvalue->closed;

        // Pop it off the open upvalue list.
        vm->openUpvalues = upvalue->next;
    }
}

static void defineMethod(VM *vm, ObjString *name) {
    Value method = peek(vm, 0);
    ObjClass *klass = AS_CLASS(peek(vm, 1));
    tableSet(vm, &klass->methods, name, method);
    pop(vm);
}

static void defineTraitMethod(VM *vm, ObjString *name) {
    Value method = peek(vm, 0);
    ObjTrait *trait = AS_TRAIT(peek(vm, 1));
    tableSet(vm, &trait->methods, name, method);
    pop(vm);
}

static void createClass(VM *vm, ObjString *name, ObjClass *superclass) {
    ObjClass *klass = newClass(vm, name, superclass);
    push(vm, OBJ_VAL(klass));

    // Inherit methods.
    if (superclass != NULL) {
        tableAddAll(vm, &superclass->methods, &klass->methods);
    }
}

bool isFalsey(Value value) {
    return IS_NIL(value) ||
           (IS_BOOL(value) && !AS_BOOL(value)) ||
           (IS_NUMBER(value) && AS_NUMBER(value) == 0) ||
           (IS_STRING(value) && AS_CSTRING(value)[0] == '\0') ||
           (IS_LIST(value) && AS_LIST(value)->values.count == 0) ||
           (IS_DICT(value) && AS_DICT(value)->count == 0) ||
           (IS_SET(value) && AS_SET(value)->count == 0);
}

static void concatenate(VM *vm) {
    ObjString *b = AS_STRING(peek(vm, 0));
    ObjString *a = AS_STRING(peek(vm, 1));

    int length = a->length + b->length;
    char *chars = ALLOCATE(vm, char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString *result = takeString(vm, chars, length);

    pop(vm);
    pop(vm);

    push(vm, OBJ_VAL(result));
}

static void setReplVar(VM *vm, Value value) {
    tableSet(vm, &vm->globals, vm->replVar, value);
}

static InterpretResult run(VM *vm) {

    CallFrame *frame = &vm->frames[vm->frameCount - 1];
    register uint8_t* ip = frame->ip;

    #define READ_BYTE() (*ip++)
    #define READ_SHORT() \
        (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))

    #define READ_CONSTANT() \
                (frame->closure->function->chunk.constants.values[READ_BYTE()])

    #define READ_STRING() AS_STRING(READ_CONSTANT())

    #define BINARY_OP(valueType, op, type) \
        do { \
          if (!IS_NUMBER(peek(vm, 0)) || !IS_NUMBER(peek(vm, 1))) { \
            frame->ip = ip; \
            runtimeError(vm, "Operands must be numbers."); \
            return INTERPRET_RUNTIME_ERROR; \
          } \
          \
          type b = AS_NUMBER(pop(vm)); \
          type a = AS_NUMBER(pop(vm)); \
          push(vm, valueType(a op b)); \
        } while (false)

    #ifdef COMPUTED_GOTO

    static void* dispatchTable[] = {
        #define OPCODE(name) &&op_##name,
        #include "opcodes.h"
        #undef OPCODE
    };

    #define INTERPRET_LOOP    DISPATCH();
    #define CASE_CODE(name)   op_##name

    #ifdef DEBUG_TRACE_EXECUTION
        #define DISPATCH()                                                                        \
            do                                                                                    \
            {                                                                                     \
                printf("          ");                                                             \
                for (Value *stackValue = vm->stack; stackValue < vm->stackTop; stackValue++) {    \
                    printf("[ ");                                                                 \
                    printValue(*stackValue);                                                      \
                    printf(" ]");                                                                 \
                }                                                                                 \
                printf("\n");                                                                     \
                disassembleInstruction(&frame->closure->function->chunk,                          \
                        (int) (frame->ip - frame->closure->function->chunk.code));                \
                goto *dispatchTable[instruction = READ_BYTE()];                                   \
            }                                                                                     \
            while (false)
    #else
        #define DISPATCH()                                            \
            do                                                        \
            {                                                         \
                goto *dispatchTable[instruction = READ_BYTE()];       \
            }                                                         \
            while (false)
    #endif

    #else

    #define INTERPRET_LOOP                                        \
            loop:                                                 \
                switch (instruction = READ_BYTE())

    #define DISPATCH() goto loop

    #define CASE_CODE(name) case OP_##name

    #endif

    uint8_t instruction;
    INTERPRET_LOOP
    {
        CASE_CODE(CONSTANT): {
            Value constant = READ_CONSTANT();
            push(vm, constant);
            DISPATCH();
        }

        CASE_CODE(NIL):
            push(vm, NIL_VAL);
            DISPATCH();

        CASE_CODE(EMPTY):
            push(vm, EMPTY_VAL);
            DISPATCH();

        CASE_CODE(TRUE):
            push(vm, BOOL_VAL(true));
            DISPATCH();

        CASE_CODE(FALSE):
            push(vm, BOOL_VAL(false));
            DISPATCH();

        CASE_CODE(POP_REPL): {
            Value v = pop(vm);
            if (!IS_NIL(v)) {
                setReplVar(vm, v);
                printValue(v);
                printf("\n");
            }
            DISPATCH();
        }

        CASE_CODE(POP): {
            pop(vm);
            DISPATCH();
        }

        CASE_CODE(GET_LOCAL): {
            uint8_t slot = READ_BYTE();
            push(vm, frame->slots[slot]);
            DISPATCH();
        }

        CASE_CODE(SET_LOCAL): {
            uint8_t slot = READ_BYTE();
            frame->slots[slot] = peek(vm, 0);
            DISPATCH();
        }

        CASE_CODE(GET_GLOBAL): {
            ObjString *name = READ_STRING();
            Value value;
            if (!tableGet(&vm->globals, name, &value)) {
                frame->ip = ip;
                runtimeError(vm, "Undefined variable '%s'.", name->chars);
                return INTERPRET_RUNTIME_ERROR;
            }
            push(vm, value);
            DISPATCH();
        }

        CASE_CODE(DEFINE_GLOBAL): {
            ObjString *name = READ_STRING();
            tableSet(vm, &vm->globals, name, peek(vm, 0));
            pop(vm);
            DISPATCH();
        }

        CASE_CODE(DEFINE_OPTIONAL): {
            // Temp array while we shuffle the stack.
            // Can not have more than 255 args to a function, so
            // we can define this with a constant limit
            Value values[255];
            int index = 0;

            values[index] = pop(vm);

            // Pop all args and default values a function has
            while (!IS_CLOSURE(values[index])) {
                values[++index] = pop(vm);
            }

            ObjClosure *closure = AS_CLOSURE(values[index--]);
            ObjFunction *function = closure->function;

            int argCount = index - function->arityOptional + 1;

            // Push the function back onto the stack
            push(vm, OBJ_VAL(closure));

            // Push all user given options
            for (int i = 0; i < argCount; i++) {
                push(vm, values[index - i]);
            }

            // Calculate how many "default" values are required
            int remaining = function->arity + function->arityOptional - argCount;

            // Push any "default" values back onto the stack
            for (int i = remaining; i > 0; i--) {
                push(vm, values[i - 1]);
            }

            DISPATCH();
        }

        CASE_CODE(SET_GLOBAL): {
            ObjString *name = READ_STRING();
            if (tableSet(vm, &vm->globals, name, peek(vm, 0))) {
                tableDelete(vm, &vm->globals, name);
                frame->ip = ip;
                runtimeError(vm, "Undefined variable '%s'.", name->chars);
                return INTERPRET_RUNTIME_ERROR;
            }
            DISPATCH();
        }

        CASE_CODE(GET_UPVALUE): {
            uint8_t slot = READ_BYTE();
            push(vm, *frame->closure->upvalues[slot]->value);
            DISPATCH();
        }

        CASE_CODE(SET_UPVALUE): {
            uint8_t slot = READ_BYTE();
            *frame->closure->upvalues[slot]->value = peek(vm, 0);
            DISPATCH();
        }

        CASE_CODE(GET_PROPERTY): {
            if (IS_INSTANCE(peek(vm, 0))) {
                ObjInstance *instance = AS_INSTANCE(peek(vm, 0));
                ObjString *name = READ_STRING();
                Value value;
                if (tableGet(&instance->fields, name, &value)) {
                    pop(vm); // Instance.
                    push(vm, value);
                    DISPATCH();
                }

                if (!bindMethod(vm, instance->klass, name)) {
                    return INTERPRET_RUNTIME_ERROR;
                }

                DISPATCH();
            } else if (IS_NATIVE_CLASS(peek(vm, 0))) {
                ObjClassNative *klass = AS_CLASS_NATIVE(peek(vm, 0));
                ObjString *name = READ_STRING();
                Value value;
                if (tableGet(&klass->properties, name, &value)) {
                    pop(vm); // Class.
                    push(vm, value);
                    DISPATCH();
                }
            }

            runtimeError(vm, "Only instances have properties.");
            return INTERPRET_RUNTIME_ERROR;
        }

        CASE_CODE(GET_PROPERTY_NO_POP): {
            if (!IS_INSTANCE(peek(vm, 0))) {
                runtimeError(vm, "Only instances have properties.");
                return INTERPRET_RUNTIME_ERROR;
            }

            ObjInstance *instance = AS_INSTANCE(peek(vm, 0));
            ObjString *name = READ_STRING();
            Value value;
            if (tableGet(&instance->fields, name, &value)) {
                push(vm, value);
                DISPATCH();
            }

            if (!bindMethod(vm, instance->klass, name)) {
                return INTERPRET_RUNTIME_ERROR;
            }

            DISPATCH();
        }

        CASE_CODE(SET_PROPERTY): {
            if (!IS_INSTANCE(peek(vm, 1))) {
                runtimeError(vm, "Only instances have fields.");
                return INTERPRET_RUNTIME_ERROR;
            }

            ObjInstance *instance = AS_INSTANCE(peek(vm, 1));
            tableSet(vm, &instance->fields, READ_STRING(), peek(vm, 0));
            pop(vm);
            pop(vm);
            push(vm, NIL_VAL);
            DISPATCH();
        }

        CASE_CODE(GET_SUPER): {
            ObjString *name = READ_STRING();
            ObjClass *superclass = AS_CLASS(pop(vm));
            if (!bindMethod(vm, superclass, name)) {
                return INTERPRET_RUNTIME_ERROR;
            }
            DISPATCH();
        }

        CASE_CODE(EQUAL): {
            Value b = pop(vm);
            Value a = pop(vm);
            push(vm, BOOL_VAL(valuesEqual(a, b)));
            DISPATCH();
        }

        CASE_CODE(GREATER):
            BINARY_OP(BOOL_VAL, >, double);
            DISPATCH();

        CASE_CODE(LESS):
            BINARY_OP(BOOL_VAL, <, double);
            DISPATCH();

        CASE_CODE(ADD): {
            if (IS_STRING(peek(vm, 0)) && IS_STRING(peek(vm, 1))) {
                concatenate(vm);
            } else if (IS_NUMBER(peek(vm, 0)) && IS_NUMBER(peek(vm, 1))) {
                double b = AS_NUMBER(pop(vm));
                double a = AS_NUMBER(pop(vm));
                push(vm, NUMBER_VAL(a + b));
            } else if (IS_LIST(peek(vm, 0)) && IS_LIST(peek(vm, 1))) {
                Value listToAddValue = peek(vm, 0);
                Value listValue = peek(vm, 1);

                ObjList *list = AS_LIST(listValue);
                ObjList *listToAdd = AS_LIST(listToAddValue);

                for (int i = 0; i < listToAdd->values.count; ++i) {
                    writeValueArray(vm, &list->values, listToAdd->values.values[i]);
                }

                pop(vm);
                pop(vm);

                push(vm, NIL_VAL);
            } else {
                frame->ip = ip;
                runtimeError(vm, "Unsupported operand types.");
                return INTERPRET_RUNTIME_ERROR;
            }
            DISPATCH();
        }

        CASE_CODE(INCREMENT): {
            if (!IS_NUMBER(peek(vm, 0))) {
                runtimeError(vm, "Operand must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }

            push(vm, NUMBER_VAL(AS_NUMBER(pop(vm)) + 1));
            DISPATCH();
        }

        CASE_CODE(DECREMENT): {
            if (!IS_NUMBER(peek(vm, 0))) {
                runtimeError(vm, "Operand must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }

            push(vm, NUMBER_VAL(AS_NUMBER(pop(vm)) - 1));
            DISPATCH();
        }

        CASE_CODE(MULTIPLY):
            BINARY_OP(NUMBER_VAL, *, double);
            DISPATCH();

        CASE_CODE(DIVIDE):
            BINARY_OP(NUMBER_VAL, /, double);
            DISPATCH();

        CASE_CODE(POW): {
            if (!IS_NUMBER(peek(vm, 0)) || !IS_NUMBER(peek(vm, 1))) {
                frame->ip = ip;
                runtimeError(vm, "Operands must be numbers.");
                return INTERPRET_RUNTIME_ERROR;
            }

            double b = AS_NUMBER(pop(vm));
            double a = AS_NUMBER(pop(vm));

            push(vm, NUMBER_VAL(powf(a, b)));
            DISPATCH();
        }

        CASE_CODE(MOD): {
            if (!IS_NUMBER(peek(vm, 0)) || !IS_NUMBER(peek(vm, 1))) {
                frame->ip = ip;
                runtimeError(vm, "Operands must be numbers.");
                return INTERPRET_RUNTIME_ERROR;
            }

            double b = AS_NUMBER(pop(vm));
            double a = AS_NUMBER(pop(vm));

            push(vm, NUMBER_VAL(fmod(a, b)));
            DISPATCH();
        }

        CASE_CODE(BITWISE_AND):
            BINARY_OP(NUMBER_VAL, &, int);
            DISPATCH();

        CASE_CODE(BITWISE_XOR):
            BINARY_OP(NUMBER_VAL, ^, int);
            DISPATCH();

        CASE_CODE(BITWISE_OR):
            BINARY_OP(NUMBER_VAL, |, int);
            DISPATCH();

        CASE_CODE(NOT):
            push(vm, BOOL_VAL(isFalsey(pop(vm))));
            DISPATCH();

        CASE_CODE(NEGATE):
            if (!IS_NUMBER(peek(vm, 0))) {
                frame->ip = ip;
                runtimeError(vm, "Operand must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }

            push(vm, NUMBER_VAL(-AS_NUMBER(pop(vm))));
            DISPATCH();

        CASE_CODE(JUMP): {
            uint16_t offset = READ_SHORT();
            ip += offset;
            DISPATCH();
        }

        CASE_CODE(JUMP_IF_FALSE): {
            uint16_t offset = READ_SHORT();
            if (isFalsey(peek(vm, 0))) ip += offset;
            DISPATCH();
        }

        CASE_CODE(LOOP): {
            uint16_t offset = READ_SHORT();
            ip -= offset;
            DISPATCH();
        }

        CASE_CODE(BREAK): {
            DISPATCH();
        }

        CASE_CODE(IMPORT): {
            ObjString *fileName = READ_STRING();

            // If we have imported this file already, skip.
            if (!tableSet(vm, &vm->imports, fileName, NIL_VAL)) {
                DISPATCH();
            }

            char *s = readFile(fileName->chars);

            if (vm->scriptNameCapacity < vm->scriptNameCount + 2) {
                int oldCapacity = vm->scriptNameCapacity;
                vm->scriptNameCapacity = GROW_CAPACITY(oldCapacity);
                vm->scriptNames = GROW_ARRAY(vm, vm->scriptNames, const char*,
                                           oldCapacity, vm->scriptNameCapacity);
            }

            vm->scriptNames[++vm->scriptNameCount] = fileName->chars;
            setcurrentFile(vm, fileName->chars, fileName->length);

            ObjFunction *function = compile(vm, s);
            if (function == NULL) return INTERPRET_COMPILE_ERROR;
            push(vm, OBJ_VAL(function));
            ObjClosure *closure = newClosure(vm, function);
            pop(vm);

            frame->ip = ip;
            call(vm, closure, 0);
            frame = &vm->frames[vm->frameCount - 1];
            ip = frame->ip;


            free(s);
            DISPATCH();
        }

        CASE_CODE(IMPORT_END): {
            vm->scriptNameCount--;
            if (vm->scriptNameCount >= 0) {
                setcurrentFile(vm, vm->scriptNames[vm->scriptNameCount],
                     (int) strlen(vm->scriptNames[vm->scriptNameCount]));
            } else {
                setcurrentFile(vm, "", 0);
            }
            DISPATCH();
        }

        CASE_CODE(NEW_LIST): {
            ObjList *list = initList(vm);
            push(vm, OBJ_VAL(list));
            DISPATCH();
        }

        CASE_CODE(ADD_LIST): {
            Value addValue = peek(vm, 0);
            Value listValue = peek(vm, 1);

            ObjList *list = AS_LIST(listValue);
            writeValueArray(vm, &list->values, addValue);

            pop(vm);
            pop(vm);

            push(vm, OBJ_VAL(list));
            DISPATCH();
        }

        CASE_CODE(NEW_DICT): {
            ObjDict *dict = initDict(vm);
            push(vm, OBJ_VAL(dict));
            DISPATCH();
        }

        CASE_CODE(ADD_DICT): {
            Value value = peek(vm, 0);
            Value key = peek(vm, 1);

            if (!isValidKey(key)) {
                runtimeError(vm, "Dictionary key must be an immutable type.");
                return INTERPRET_RUNTIME_ERROR;
            }

            ObjDict *dict = AS_DICT(peek(vm, 2));
            dictSet(vm, dict, key, value);

            pop(vm);
            pop(vm);
            pop(vm);

            push(vm, OBJ_VAL(dict));
            DISPATCH();
        }

        CASE_CODE(SUBSCRIPT): {
            Value indexValue = pop(vm);
            Value subscriptValue = pop(vm);

            if (!IS_OBJ(subscriptValue)) {
                frame->ip = ip;
                runtimeError(vm, "Can only subscript on lists, strings or dictionaries.");
                return INTERPRET_RUNTIME_ERROR;
            }

            switch (getObjType(subscriptValue)) {
                case OBJ_LIST: {
                    if (!IS_NUMBER(indexValue)) {
                        frame->ip = ip;
                        runtimeError(vm, "List index must be a number.");
                        return INTERPRET_RUNTIME_ERROR;
                    }

                    ObjList *list = AS_LIST(subscriptValue);
                    int index = AS_NUMBER(indexValue);

                    // Allow negative indexes
                    if (index < 0)
                        index = list->values.count + index;

                    if (index >= 0 && index < list->values.count) {
                        push(vm, list->values.values[index]);
                        DISPATCH();
                    }

                    frame->ip = ip;
                    runtimeError(vm, "List index out of bounds.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                case OBJ_STRING: {
                    ObjString *string = AS_STRING(subscriptValue);
                    int index = AS_NUMBER(indexValue);

                    // Allow negative indexes
                    if (index < 0)
                        index = string->length + index;

                    if (index >= 0 && index < string->length) {
                        push(vm, OBJ_VAL(copyString(vm, &string->chars[index], 1)));
                        DISPATCH();
                    }

                    frame->ip = ip;
                    runtimeError(vm, "String index out of bounds.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                case OBJ_DICT: {
                    ObjDict *dict = AS_DICT(subscriptValue);
                    if (!isValidKey(indexValue)) {
                        runtimeError(vm, "Dictionary key must be an immutable type.");
                        return INTERPRET_RUNTIME_ERROR;
                    }

                    Value v;
                    if (dictGet(dict, indexValue, &v)) {
                        push(vm, v);
                    } else {
                        push(vm, NIL_VAL);
                    }

                    DISPATCH();
                }

                default: {
                    frame->ip = ip;
                    runtimeError(vm, "Can only subscript on lists, strings or dictionaries.");
                    return INTERPRET_RUNTIME_ERROR;
                }
            }
        }

        CASE_CODE(SUBSCRIPT_ASSIGN): {
            Value assignValue = peek(vm, 0);
            Value indexValue = peek(vm, 1);
            Value subscriptValue = peek(vm, 2);

            if (!IS_OBJ(subscriptValue)) {
                frame->ip = ip;
                runtimeError(vm, "Can only subscript on lists, strings or dictionaries.");
                return INTERPRET_RUNTIME_ERROR;
            }

            switch (getObjType(subscriptValue)) {
                case OBJ_LIST: {
                    if (!IS_NUMBER(indexValue)) {
                        frame->ip = ip;
                        runtimeError(vm, "List index must be a number.");
                        return INTERPRET_RUNTIME_ERROR;
                    }

                    ObjList *list = AS_LIST(subscriptValue);
                    int index = AS_NUMBER(indexValue);

                    if (index < 0)
                        index = list->values.count + index;

                    if (index >= 0 && index < list->values.count) {
                        list->values.values[index] = assignValue;
                        push(vm, NIL_VAL);
                        DISPATCH();
                    }

                    // Pop after the values have been inserted to stop GC cleanup
                    pop(vm);
                    pop(vm);
                    pop(vm);

                    push(vm, NIL_VAL);

                    frame->ip = ip;
                    runtimeError(vm, "List index out of bounds.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                case OBJ_DICT: {
                    ObjDict *dict = AS_DICT(subscriptValue);
                    if (!isValidKey(indexValue)) {
                        runtimeError(vm, "Dictionary key must be an immutable type.");
                        return INTERPRET_RUNTIME_ERROR;
                    }

                    dictSet(vm, dict, indexValue, assignValue);

                    // Pop after the values have been inserted to stop GC cleanup
                    pop(vm);
                    pop(vm);
                    pop(vm);

                    push(vm, NIL_VAL);
                    DISPATCH();
                }

                default: {
                    pop(vm);
                    pop(vm);
                    pop(vm);

                    frame->ip = ip;
                    runtimeError(vm, "Only lists and dictionaries support subscript assignment.");
                    return INTERPRET_RUNTIME_ERROR;
                }
            }
        }

        CASE_CODE(SLICE): {
            Value sliceEndIndex = peek(vm, 0);
            Value sliceStartIndex = peek(vm, 1);
            Value objectValue = peek(vm, 2);

            if (!IS_OBJ(objectValue)) {
                frame->ip = ip;
                runtimeError(vm, "Can only slice on lists and strings.");
                return INTERPRET_RUNTIME_ERROR;
            }

            if ((!IS_NUMBER(sliceStartIndex) && !IS_EMPTY(sliceStartIndex)) || (!IS_NUMBER(sliceEndIndex) && !IS_EMPTY(sliceEndIndex))) {
                frame->ip = ip;
                runtimeError(vm, "Slice index must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }

            int indexStart;
            int indexEnd;
            Value returnVal;

            if (IS_EMPTY(sliceStartIndex)) {
                indexStart = 0;
            } else {
                indexStart = AS_NUMBER(sliceStartIndex);

                if (indexStart < 0) {
                    indexStart = 0;
                }
            }

            switch (getObjType(objectValue)) {
                case OBJ_LIST: {
                    ObjList *newList = initList(vm);
                    push(vm, OBJ_VAL(newList));
                    ObjList *list = AS_LIST(objectValue);

                    if (IS_EMPTY(sliceEndIndex)) {
                        indexEnd = list->values.count;
                    } else {
                        indexEnd = AS_NUMBER(sliceEndIndex);

                        if (indexEnd > list->values.count) {
                            indexEnd = list->values.count;
                        }
                    }

                    for (int i = indexStart; i < indexEnd; i++) {
                        writeValueArray(vm, &newList->values, list->values.values[i]);
                    }

                    pop(vm);
                    returnVal = OBJ_VAL(newList);

                    break;
                }

                case OBJ_STRING: {
                    ObjString *string = AS_STRING(objectValue);

                    if (IS_EMPTY(sliceEndIndex)) {
                        indexEnd = string->length;
                    } else {
                        indexEnd = AS_NUMBER(sliceEndIndex);

                        if (indexEnd > string->length) {
                            indexEnd = string->length;
                        }
                    }

                    // Ensure the start index is below the end index
                    if (indexStart > indexEnd) {
                        returnVal = OBJ_VAL(copyString(vm, "", 0));
                    } else {
                        char *newString = malloc(sizeof(char) * (indexEnd - indexStart) + 1);
                        memcpy(newString, string->chars + indexStart, indexEnd - indexStart);
                        returnVal = OBJ_VAL(copyString(vm, newString, indexEnd - indexStart));
                        free(newString);
                    }
                    break;
                }

                default: {
                    frame->ip = ip;
                    runtimeError(vm, "Can only slice on lists and strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
            }

            pop(vm);
            pop(vm);
            pop(vm);

            push(vm, returnVal);
            DISPATCH();
        }

        CASE_CODE(PUSH): {
            Value value = pop(vm);
            Value indexValue = pop(vm);
            Value subscriptValue = pop(vm);

            if (!IS_OBJ(subscriptValue)) {
                frame->ip = ip;
                runtimeError(vm, "Can only subscript on lists, strings or dictionaries.");
                return INTERPRET_RUNTIME_ERROR;
            }

            switch (getObjType(subscriptValue)) {
                case OBJ_LIST: {
                    if (!IS_NUMBER(indexValue)) {
                        frame->ip = ip;
                        runtimeError(vm, "List index must be a number.");
                        return INTERPRET_RUNTIME_ERROR;
                    }

                    ObjList *list = AS_LIST(subscriptValue);
                    int index = AS_NUMBER(indexValue);

                    // Allow negative indexes
                    if (index < 0)
                        index = list->values.count + index;

                    if (index >= 0 && index < list->values.count) {
                        push(vm, subscriptValue);
                        push(vm, indexValue);
                        push(vm, list->values.values[index]);
                        push(vm, value);
                        DISPATCH();
                    }

                    frame->ip = ip;
                    runtimeError(vm, "List index out of bounds.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                case OBJ_DICT: {
                    ObjDict *dict = AS_DICT(subscriptValue);
                    if (!isValidKey(indexValue)) {
                        runtimeError(vm, "Dictionary key must be an immutable type.");
                        return INTERPRET_RUNTIME_ERROR;
                    }

                    Value v;
                    bool found = dictGet(dict, indexValue, &v);

                    push(vm, subscriptValue);
                    push(vm, indexValue);
                    if (found) {
                        push(vm, v);
                    } else {
                        push(vm, NIL_VAL);
                    }

                    push(vm, value);
                    DISPATCH();
                }

                default: {
                    frame->ip = ip;
                    runtimeError(vm, "Only lists and dictionaries support subscript assignment.");
                    return INTERPRET_RUNTIME_ERROR;
                }
            }
            DISPATCH();
        }

        CASE_CODE(CALL): {
            int argCount = READ_BYTE();
            frame->ip = ip;
            if (!callValue(vm, peek(vm, argCount), argCount)) {
                return INTERPRET_RUNTIME_ERROR;
            }
            frame = &vm->frames[vm->frameCount - 1];
            ip = frame->ip;
            DISPATCH();
        }

        CASE_CODE(INVOKE): {
            int argCount = READ_BYTE();
            ObjString *method = READ_STRING();
            frame->ip = ip;
            if (!invoke(vm, method, argCount)) {
                return INTERPRET_RUNTIME_ERROR;
            }
            frame = &vm->frames[vm->frameCount - 1];
            ip = frame->ip;
            DISPATCH();
        }

        CASE_CODE(SUPER): {
            int argCount = READ_BYTE();
            ObjString *method = READ_STRING();
            frame->ip = ip;
            ObjClass *superclass = AS_CLASS(pop(vm));
            if (!invokeFromClass(vm, superclass, method, argCount)) {
                return INTERPRET_RUNTIME_ERROR;
            }
            frame = &vm->frames[vm->frameCount - 1];
            ip = frame->ip;
            DISPATCH();
        }

        CASE_CODE(CLOSURE): {
            ObjFunction *function = AS_FUNCTION(READ_CONSTANT());

            // Create the closure and push it on the stack before creating
            // upvalues so that it doesn't get collected.
            ObjClosure *closure = newClosure(vm, function);
            push(vm, OBJ_VAL(closure));

            // Capture upvalues.
            for (int i = 0; i < closure->upvalueCount; i++) {
                uint8_t isLocal = READ_BYTE();
                uint8_t index = READ_BYTE();
                if (isLocal) {
                    // Make an new upvalue to close over the parent's local
                    // variable.
                    closure->upvalues[i] = captureUpvalue(vm, frame->slots + index);
                } else {
                    // Use the same upvalue as the current call frame.
                    closure->upvalues[i] = frame->closure->upvalues[index];
                }
            }

            DISPATCH();
        }

        CASE_CODE(CLOSE_UPVALUE): {
            closeUpvalues(vm, vm->stackTop - 1);
            pop(vm);
            DISPATCH();
        }

        CASE_CODE(RETURN): {
            Value result = pop(vm);

            // Close any upvalues still in scope.
            closeUpvalues(vm, frame->slots);

            vm->frameCount--;

            if (vm->frameCount == 0) {
                pop(vm);
                return INTERPRET_OK;
            }

            vm->stackTop = frame->slots;
            push(vm, result);

            frame = &vm->frames[vm->frameCount - 1];
            ip = frame->ip;
            DISPATCH();
        }

        CASE_CODE(CLASS):
            createClass(vm, READ_STRING(), NULL);
            DISPATCH();

        CASE_CODE(SUBCLASS): {
            Value superclass = peek(vm, 0);
            if (!IS_CLASS(superclass)) {
                frame->ip = ip;
                runtimeError(vm, "Superclass must be a class.");
                return INTERPRET_RUNTIME_ERROR;
            }

            createClass(vm, READ_STRING(), AS_CLASS(superclass));
            DISPATCH();
        }

        CASE_CODE(TRAIT): {
            ObjString *name = READ_STRING();
            ObjTrait *trait = newTrait(vm, name);
            push(vm, OBJ_VAL(trait));
            DISPATCH();
        }

        CASE_CODE(METHOD):
            defineMethod(vm, READ_STRING());
            DISPATCH();

        CASE_CODE(TRAIT_METHOD): {
            defineTraitMethod(vm, READ_STRING());
            DISPATCH();
        }

        CASE_CODE(USE): {
            Value trait = peek(vm, 0);
            if (!IS_TRAIT(trait)) {
                frame->ip = ip;
                runtimeError(vm, "Can only 'use' with a trait");
                return INTERPRET_RUNTIME_ERROR;
            }

            ObjClass *klass = AS_CLASS(peek(vm, 1));

            tableAddAll(vm, &AS_TRAIT(trait)->methods, &klass->methods);
            pop(vm); // pop the trait

            DISPATCH();
        }

        CASE_CODE(OPEN_FILE): {
            Value openType = pop(vm);
            Value fileName = pop(vm);

            if (!IS_STRING(openType)) {
                frame->ip = ip;
                runtimeError(vm, "File open type must be a string");
                return INTERPRET_RUNTIME_ERROR;
            }

            if (!IS_STRING(fileName)) {
                frame->ip = ip;
                runtimeError(vm, "Filename must be a string");
                return INTERPRET_RUNTIME_ERROR;
            }

            ObjString *openTypeString = AS_STRING(openType);
            ObjString *fileNameString = AS_STRING(fileName);

            ObjFile *file = initFile(vm);
            file->file = fopen(fileNameString->chars, openTypeString->chars);
            file->path = fileNameString->chars;
            file->openType = openTypeString->chars;

            if (file->file == NULL) {
                frame->ip = ip;
                runtimeError(vm, "Unable to open file");
                return INTERPRET_RUNTIME_ERROR;
            }

            push(vm, OBJ_VAL(file));
            DISPATCH();
        }

        CASE_CODE(CLOSE_FILE): {
            ObjFile *file = AS_FILE(peek(vm, 0));
            fclose(file->file);
            DISPATCH();
        }
    }

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP

    return INTERPRET_RUNTIME_ERROR;

}

InterpretResult interpret(VM *vm, const char *source) {
    ObjFunction *function = compile(vm, source);
    if (function == NULL) return INTERPRET_COMPILE_ERROR;
    push(vm, OBJ_VAL(function));
    ObjClosure *closure = newClosure(vm, function);
    pop(vm);
    push(vm, OBJ_VAL(closure));
    callValue(vm, OBJ_VAL(closure), 0);
    InterpretResult result = run(vm);

    return result;
}
