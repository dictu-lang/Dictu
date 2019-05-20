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
#include "collections.h"
#include "strings.h"
#include "files.h"
#include "natives.h"

VM vm; // [one]

void defineAllNatives();

static void resetStack() {
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
    vm.currentFrameCount = 0;
    vm.openUpvalues = NULL;
}

void runtimeError(const char *format, ...) {
    for (int i = vm.frameCount - 1; i >= 0; i--) {
        CallFrame *frame = &vm.frames[i];

        ObjFunction *function = frame->closure->function;

        // -1 because the IP is sitting on the next instruction to be
        // executed.
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[line %d] in ",
                function->chunk.lines[instruction]);

        if (function->name == NULL) {
            fprintf(stderr, "%s: ", vm.currentScriptName);
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

    resetStack();
}

void initVM(bool repl, const char *scriptName) {
    resetStack();
    vm.objects = NULL;
    vm.listObjects = NULL;
    vm.repl = repl;
    vm.scriptName = scriptName;
    vm.currentScriptName = scriptName;
    vm.bytesAllocated = 0;
    vm.nextGC = 1024 * 1024;
    vm.grayCount = 0;
    vm.grayCapacity = 0;
    vm.grayStack = NULL;
    initTable(&vm.globals);
    initTable(&vm.strings);
    vm.initString = copyString("init", 4);
    vm.replVar = copyString("_", 1);
    defineAllNatives();
}

void freeVM() {
    freeTable(&vm.globals);
    freeTable(&vm.strings);
    vm.initString = NULL;
    vm.replVar = NULL;
    freeObjects();
    freeLists();
}

void push(Value value) {
    *vm.stackTop = value;
    vm.stackTop++;
    vm.stackCount++;
}

Value pop() {
    vm.stackTop--;
    vm.stackCount--;
    return *vm.stackTop;
}

Value peek(int distance) {
    return vm.stackTop[-1 - distance];
}

static bool call(ObjClosure *closure, int argCount) {
    if (argCount != closure->function->arity) {
        runtimeError("Function '%s' expected %d arguments but got %d.",
                     closure->function->name->chars,
                     closure->function->arity,
                     argCount
        );

        return false;
    }

    if (vm.frameCount == FRAMES_MAX) {
        runtimeError("Stack overflow.");
        return false;
    }

    CallFrame *frame = &vm.frames[vm.frameCount++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;

    // +1 to include either the called function or the receiver.
    frame->slots = vm.stackTop - (argCount + 1);
    return true;
}

static bool callValue(Value callee, int argCount) {
    if (IS_OBJ(callee)) {
        switch (OBJ_TYPE(callee)) {
            case OBJ_BOUND_METHOD: {
                ObjBoundMethod *bound = AS_BOUND_METHOD(callee);

                // Replace the bound method with the receiver so it's in the
                // right slot when the method is called.
                vm.stackTop[-argCount - 1] = bound->receiver;
                return call(bound->method, argCount);
            }

            case OBJ_CLASS: {
                ObjClass *klass = AS_CLASS(callee);

                // Create the instance.
                vm.stackTop[-argCount - 1] = OBJ_VAL(newInstance(klass));

                // Call the initializer, if there is one.
                Value initializer;
                if (tableGet(&klass->methods, vm.initString, &initializer)) {
                    return call(AS_CLOSURE(initializer), argCount);
                } else if (argCount != 0) {
                    runtimeError("Expected 0 arguments but got %d.", argCount);
                    return false;
                }

                return true;
            }

            case OBJ_CLOSURE:
                return call(AS_CLOSURE(callee), argCount);

            case OBJ_NATIVE_VOID: {
                NativeFnVoid native = AS_NATIVE_VOID(callee);
                if (!native(argCount, vm.stackTop - argCount))
                    return false;

                vm.stackTop -= argCount + 1;
                vm.stackCount -= argCount + 1;
                push(NIL_VAL);
                return true;
            }

            case OBJ_NATIVE: {
                NativeFn native = AS_NATIVE(callee);
                Value result = native(argCount, vm.stackTop - argCount);

                if (IS_NIL(result))
                    return false;

                vm.stackTop -= argCount + 1;
                vm.stackCount -= argCount + 1;
                push(result);
                return true;
            }

            default:
                // Do nothing.
                break;
        }
    }

    runtimeError("Can only call functions and classes.");
    return false;
}

static bool invokeFromClass(ObjClass *klass, ObjString *name,
                            int argCount) {
    // Look for the method.
    Value method;
    if (!tableGet(&klass->methods, name, &method)) {
        runtimeError("Undefined property '%s'.", name->chars);
        return false;
    }

    return call(AS_CLOSURE(method), argCount);
}

static bool invoke(ObjString *name, int argCount) {
    Value receiver = peek(argCount);

    if (IS_CLASS(receiver)) {
        ObjClass *instance = AS_CLASS(receiver);
        Value method;
        if (!tableGet(&instance->methods, name, &method)) {
            runtimeError("Undefined property '%s'.", name->chars);
            return false;
        }

        if (!AS_CLOSURE(method)->function->staticMethod) {
            runtimeError("'%s' is not static. Only static methods can be invoked directly from a class.", name->chars);
            return false;
        }

        return callValue(method, argCount);
    } else if (IS_LIST(receiver)) {
        return listMethods(name->chars, argCount + 1);
    } else if (IS_DICT(receiver)) {
        return dictMethods(name->chars, argCount + 1);
    } else if (IS_STRING(receiver)) {
        return stringMethods(name->chars, argCount + 1);
    } else if (IS_FILE(receiver)) {
        return fileMethods(name->chars, argCount + 1);
    }

    if (!IS_INSTANCE(receiver)) {
        runtimeError("Only instances have methods.");
        return false;
    }

    ObjInstance *instance = AS_INSTANCE(receiver);

    // First look for a field which may shadow a method.
    Value value;
    if (tableGet(&instance->fields, name, &value)) {
        vm.stackTop[-argCount] = value;
        return callValue(value, argCount);
    }

    return invokeFromClass(instance->klass, name, argCount);
}

static bool bindMethod(ObjClass *klass, ObjString *name) {
    Value method;
    if (!tableGet(&klass->methods, name, &method)) {
        runtimeError("Undefined property '%s'.", name->chars);
        return false;
    }

    ObjBoundMethod *bound = newBoundMethod(peek(0), AS_CLOSURE(method));
    pop(); // Instance.
    push(OBJ_VAL(bound));
    return true;
}

// Captures the local variable [local] into an [Upvalue]. If that local
// is already in an upvalue, the existing one is used. (This is
// important to ensure that multiple closures closing over the same
// variable actually see the same variable.) Otherwise, it creates a
// new open upvalue and adds it to the VM's list of upvalues.
static ObjUpvalue *captureUpvalue(Value *local) {
    // If there are no open upvalues at all, we must need a new one.
    if (vm.openUpvalues == NULL) {
        vm.openUpvalues = newUpvalue(local);
        return vm.openUpvalues;
    }

    ObjUpvalue *prevUpvalue = NULL;
    ObjUpvalue *upvalue = vm.openUpvalues;

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
    ObjUpvalue *createdUpvalue = newUpvalue(local);
    createdUpvalue->next = upvalue;

    if (prevUpvalue == NULL) {
        // The new one is the first one in the list.
        vm.openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->next = createdUpvalue;
    }

    return createdUpvalue;
}

static void closeUpvalues(Value *last) {
    while (vm.openUpvalues != NULL &&
           vm.openUpvalues->value >= last) {
        ObjUpvalue *upvalue = vm.openUpvalues;

        // Move the value into the upvalue itself and point the upvalue to
        // it.
        upvalue->closed = *upvalue->value;
        upvalue->value = &upvalue->closed;

        // Pop it off the open upvalue list.
        vm.openUpvalues = upvalue->next;
    }
}

static void defineMethod(ObjString *name) {
    Value method = peek(0);
    ObjClass *klass = AS_CLASS(peek(1));
    tableSet(&klass->methods, name, method);
    pop();
}

static void createClass(ObjString *name, ObjClass *superclass) {
    ObjClass *klass = newClass(name, superclass);
    push(OBJ_VAL(klass));

    // Inherit methods.
    if (superclass != NULL) {
        tableAddAll(&superclass->methods, &klass->methods);
    }
}

bool isFalsey(Value value) {
    return IS_NIL(value) ||
           (IS_BOOL(value) && !AS_BOOL(value)) ||
           (IS_NUMBER(value) && AS_NUMBER(value) == 0) ||
           (IS_STRING(value) && AS_CSTRING(value)[0] == '\0');
}

static void concatenate() {
    ObjString *b = AS_STRING(peek(0));
    ObjString *a = AS_STRING(peek(1));

    int length = a->length + b->length;
    char *chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString *result = takeString(chars, length);

    pop();
    pop();

    push(OBJ_VAL(result));
}

static void setReplVar(Value value) {
    tableSet(&vm.globals, vm.replVar, value);
}

static InterpretResult run() {

    CallFrame *frame = &vm.frames[vm.frameCount - 1];

    #define READ_BYTE() (*frame->ip++)
    #define READ_SHORT() \
                (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

    #define READ_CONSTANT() \
                (frame->closure->function->chunk.constants.values[READ_BYTE()])

    #define READ_STRING() AS_STRING(READ_CONSTANT())

    #define BINARY_OP(valueType, op) \
        do { \
          if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            runtimeError("Operands must be numbers."); \
            return INTERPRET_RUNTIME_ERROR; \
          } \
          \
          double b = AS_NUMBER(pop()); \
          double a = AS_NUMBER(pop()); \
          push(valueType(a op b)); \
        } while (false)

    #ifdef COMPUTED_GOTO

    static void* dispatchTable[] = {
        #define OPCODE(name) &&op_##name,
        #include "opcodes.h"
        #undef OPCODE
    };

    #define INTERPRET_LOOP    DISPATCH();
    #define CASE_CODE(name)   op_##name

    #define DISPATCH()                                            \
        do                                                        \
        {                                                         \
            goto *dispatchTable[instruction = READ_BYTE()];       \
        }                                                         \
        while (false)

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
            push(constant);
            DISPATCH();
        }

        CASE_CODE(NIL):
            push(NIL_VAL);
            DISPATCH();

        CASE_CODE(TRUE):
            push(BOOL_VAL(true));
            DISPATCH();

        CASE_CODE(FALSE):
            push(BOOL_VAL(false));
            DISPATCH();

        CASE_CODE(POP_REPL): {
            if (vm.repl) {
                Value v = pop();
                if (!IS_NIL(v)) {
                    setReplVar(v);
                    printValue(v);
                    printf("\n");
                }
            } else {
                pop();
            }
            DISPATCH();
        }

        CASE_CODE(POP): {
            if (IS_FILE(peek(0))) {
                ObjFile *file = AS_FILE(peek(0));
                fclose(file->file);
                collectGarbage();
            }

            pop();
            DISPATCH();
        }

        CASE_CODE(GET_LOCAL): {
            uint8_t slot = READ_BYTE();
            push(frame->slots[slot]);
            DISPATCH();
        }

        CASE_CODE(SET_LOCAL): {
            uint8_t slot = READ_BYTE();
            frame->slots[slot] = peek(0);
            DISPATCH();
        }

        CASE_CODE(GET_GLOBAL): {
            ObjString *name = READ_STRING();
            Value value;
            if (!tableGet(&vm.globals, name, &value)) {
                runtimeError("Undefined variable '%s'.", name->chars);
                return INTERPRET_RUNTIME_ERROR;
            }
            push(value);
            DISPATCH();
        }

        CASE_CODE(DEFINE_GLOBAL): {
            ObjString *name = READ_STRING();
            tableSet(&vm.globals, name, peek(0));
            pop();
            DISPATCH();
        }

        CASE_CODE(SET_GLOBAL): {
            ObjString *name = READ_STRING();
            if (tableSet(&vm.globals, name, peek(0))) {
                runtimeError("Undefined variable '%s'.", name->chars);
                return INTERPRET_RUNTIME_ERROR;
            }
            DISPATCH();
        }

        CASE_CODE(GET_UPVALUE): {
            uint8_t slot = READ_BYTE();
            push(*frame->closure->upvalues[slot]->value);
            DISPATCH();
        }

        CASE_CODE(SET_UPVALUE): {
            uint8_t slot = READ_BYTE();
            *frame->closure->upvalues[slot]->value = peek(0);
            DISPATCH();
        }

        CASE_CODE(GET_PROPERTY): {
            if (!IS_INSTANCE(peek(0))) {
                runtimeError("Only instances have properties.");
                return INTERPRET_RUNTIME_ERROR;
            }

            ObjInstance *instance = AS_INSTANCE(peek(0));
            ObjString *name = READ_STRING();
            Value value;
            if (tableGet(&instance->fields, name, &value)) {
                pop(); // Instance.
                push(value);
                DISPATCH();
            }

            if (!bindMethod(instance->klass, name)) {
                return INTERPRET_RUNTIME_ERROR;
            }

            DISPATCH();
        }

        CASE_CODE(GET_PROPERTY_NO_POP): {
            if (!IS_INSTANCE(peek(0))) {
                runtimeError("Only instances have properties.");
                return INTERPRET_RUNTIME_ERROR;
            }

            ObjInstance *instance = AS_INSTANCE(peek(0));
            ObjString *name = READ_STRING();
            Value value;
            if (tableGet(&instance->fields, name, &value)) {
                push(value);
                DISPATCH();
            }

            if (!bindMethod(instance->klass, name)) {
                return INTERPRET_RUNTIME_ERROR;
            }

            DISPATCH();
        }

        CASE_CODE(SET_PROPERTY): {
            if (!IS_INSTANCE(peek(1))) {
                runtimeError("Only instances have fields.");
                return INTERPRET_RUNTIME_ERROR;
            }

            ObjInstance *instance = AS_INSTANCE(peek(1));
            tableSet(&instance->fields, READ_STRING(), peek(0));
            pop();
            pop();
            push(NIL_VAL);
            DISPATCH();
        }

        CASE_CODE(GET_SUPER): {
            ObjString *name = READ_STRING();
            ObjClass *superclass = AS_CLASS(pop());
            if (!bindMethod(superclass, name)) {
                return INTERPRET_RUNTIME_ERROR;
            }
            DISPATCH();
        }

        CASE_CODE(EQUAL): {
            Value b = pop();
            Value a = pop();
            push(BOOL_VAL(valuesEqual(a, b)));
            DISPATCH();
        }

        CASE_CODE(GREATER):
            BINARY_OP(BOOL_VAL, >);
            DISPATCH();

        CASE_CODE(LESS):
            BINARY_OP(BOOL_VAL, <);
            DISPATCH();

        CASE_CODE(ADD): {
            if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                concatenate();
            } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                push(NUMBER_VAL(a + b));
            } else if (IS_LIST(peek(0)) && IS_LIST(peek(1))) {
                Value listToAddValue = pop();
                Value listValue = pop();

                ObjList *list = AS_LIST(listValue);
                ObjList *listToAdd = AS_LIST(listToAddValue);

                for (int i = 0; i < listToAdd->values.count; ++i) {
                    writeValueArray(&list->values, listToAdd->values.values[i]);
                }

                push(NIL_VAL);
            } else {
                runtimeError("Unsupported operand types.");
                return INTERPRET_RUNTIME_ERROR;
            }
            DISPATCH();
        }

        CASE_CODE(SUBTRACT):
            BINARY_OP(NUMBER_VAL, -);
            DISPATCH();

        CASE_CODE(INCREMENT): {
            if (!IS_NUMBER(peek(0))) {
                runtimeError("Operand must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }

            push(NUMBER_VAL(AS_NUMBER(pop()) + 1));
            DISPATCH();
        }

        CASE_CODE(DECREMENT): {
            if (!IS_NUMBER(peek(0))) {
                runtimeError("Operand must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }

            push(NUMBER_VAL(AS_NUMBER(pop()) - 1));
            DISPATCH();
        }

        CASE_CODE(MULTIPLY):
            BINARY_OP(NUMBER_VAL, *);
            DISPATCH();

        CASE_CODE(DIVIDE):
            BINARY_OP(NUMBER_VAL, /);
            DISPATCH();

        CASE_CODE(MOD): {
            double b = AS_NUMBER(pop());
            double a = AS_NUMBER(pop());

            push(NUMBER_VAL(fmod(a, b)));
            DISPATCH();
        }

        CASE_CODE(NOT):
            push(BOOL_VAL(isFalsey(pop())));
            DISPATCH();

        CASE_CODE(NEGATE):
            if (!IS_NUMBER(peek(0))) {
                runtimeError("Operand must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }

            push(NUMBER_VAL(-AS_NUMBER(pop())));
            DISPATCH();

        CASE_CODE(JUMP): {
            uint16_t offset = READ_SHORT();
            frame->ip += offset;
            DISPATCH();
        }

        CASE_CODE(JUMP_IF_FALSE): {
            uint16_t offset = READ_SHORT();
            if (isFalsey(peek(0))) frame->ip += offset;
            DISPATCH();
        }

        CASE_CODE(LOOP): {
            uint16_t offset = READ_SHORT();
            frame->ip -= offset;
            DISPATCH();
        }

        CASE_CODE(BREAK): {

            DISPATCH();
        }

        CASE_CODE(IMPORT): {
            ObjString *fileName = AS_STRING(pop());
            char *s = readFile(fileName->chars);
            vm.currentScriptName = fileName->chars;

            ObjFunction *function = compile(s);
            if (function == NULL) return INTERPRET_COMPILE_ERROR;
            push(OBJ_VAL(function));
            ObjClosure *closure = newClosure(function);
            pop();

            vm.currentFrameCount = vm.frameCount;

            frame = &vm.frames[vm.frameCount++];
            frame->ip = closure->function->chunk.code;
            frame->closure = closure;
            frame->slots = vm.stackTop - 1;
            free(s);
            DISPATCH();
        }

        CASE_CODE(NEW_LIST): {
            ObjList *list = initList();
            push(OBJ_VAL(list));
            DISPATCH();
        }

        CASE_CODE(ADD_LIST): {
            Value addValue = pop();
            Value listValue = pop();

            ObjList *list = AS_LIST(listValue);
            writeValueArray(&list->values, addValue);

            push(OBJ_VAL(list));
            DISPATCH();
        }

        CASE_CODE(NEW_DICT): {
            ObjDict *dict = initDict();
            push(OBJ_VAL(dict));
            DISPATCH();
        }

        CASE_CODE(ADD_DICT): {
            Value value = pop();
            Value key = pop();
            Value dictValue = pop();

            if (!IS_STRING(key)) {
                runtimeError("Dictionary key must be a string.");
                return INTERPRET_RUNTIME_ERROR;
            }

            ObjDict *dict = AS_DICT(dictValue);
            char *keyString = AS_CSTRING(key);

            insertDict(dict, keyString, value);

            push(OBJ_VAL(dict));
            DISPATCH();
        }

        CASE_CODE(SUBSCRIPT): {
            Value indexValue = pop();
            Value listValue = pop();

            if (!IS_NUMBER(indexValue)) {
                runtimeError("Array index must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }

            ObjList *list = AS_LIST(listValue);
            int index = AS_NUMBER(indexValue);

            if (index < 0)
                index = list->values.count + index;

            if (index >= 0 && index < list->values.count) {
                push(list->values.values[index]);
                DISPATCH();
            }

            runtimeError("Array index out of bounds.");
            return INTERPRET_RUNTIME_ERROR;
        }

        CASE_CODE(SUBSCRIPT_ASSIGN): {
            Value assignValue = pop();
            Value indexValue = pop();
            Value listValue = pop();

            if (!IS_NUMBER(indexValue)) {
                runtimeError("Array index must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }

            ObjList *list = AS_LIST(listValue);
            int index = AS_NUMBER(indexValue);

            if (index < 0)
                index = list->values.count + index;

            if (index >= 0 && index < list->values.count) {
                list->values.values[index] = assignValue;
                push(NIL_VAL);
                DISPATCH();
            }

            push(NIL_VAL);

            runtimeError("Array index out of bounds.");
            return INTERPRET_RUNTIME_ERROR;
        }

        CASE_CODE(SUBSCRIPT_DICT): {
            Value indexValue = pop();
            Value dictValue = pop();

            if (!IS_STRING(indexValue)) {
                runtimeError("Dictionary key must be a string.");
                return INTERPRET_RUNTIME_ERROR;
            }

            ObjDict *dict = AS_DICT(dictValue);
            char *key = AS_CSTRING(indexValue);

            push(searchDict(dict, key));

            DISPATCH();
        }

        CASE_CODE(SUBSCRIPT_DICT_ASSIGN): {
            Value value = pop();
            Value key = pop();
            Value dictValue = pop();

            if (!IS_STRING(key)) {
                runtimeError("Dictionary key must be a string.");
                return INTERPRET_RUNTIME_ERROR;
            }

            ObjDict *dict = AS_DICT(dictValue);
            char *keyString = AS_CSTRING(key);

            insertDict(dict, keyString, value);

            push(NIL_VAL);
            DISPATCH();
        }

        CASE_CODE(CALL_0):
        CASE_CODE(CALL_1):
        CASE_CODE(CALL_2):
        CASE_CODE(CALL_3):
        CASE_CODE(CALL_4):
        CASE_CODE(CALL_5):
        CASE_CODE(CALL_6):
        CASE_CODE(CALL_7):
        CASE_CODE(CALL_8):
        CASE_CODE(CALL_9):
        CASE_CODE(CALL_10):
        CASE_CODE(CALL_11):
        CASE_CODE(CALL_12):
        CASE_CODE(CALL_13):
        CASE_CODE(CALL_14):
        CASE_CODE(CALL_15):
        CASE_CODE(CALL_16):
        CASE_CODE(CALL_17):
        CASE_CODE(CALL_18):
        CASE_CODE(CALL_19):
        CASE_CODE(CALL_20):
        CASE_CODE(CALL_21):
        CASE_CODE(CALL_22):
        CASE_CODE(CALL_23):
        CASE_CODE(CALL_24):
        CASE_CODE(CALL_25):
        CASE_CODE(CALL_26):
        CASE_CODE(CALL_27):
        CASE_CODE(CALL_28):
        CASE_CODE(CALL_29):
        CASE_CODE(CALL_30):
        CASE_CODE(CALL_31): {
            int argCount = instruction - OP_CALL_0;
            if (!callValue(peek(argCount), argCount)) {
                return INTERPRET_RUNTIME_ERROR;
            }
            frame = &vm.frames[vm.frameCount - 1];
            DISPATCH();
        }

        CASE_CODE(INVOKE_0):
        CASE_CODE(INVOKE_1):
        CASE_CODE(INVOKE_2):
        CASE_CODE(INVOKE_3):
        CASE_CODE(INVOKE_4):
        CASE_CODE(INVOKE_5):
        CASE_CODE(INVOKE_6):
        CASE_CODE(INVOKE_7):
        CASE_CODE(INVOKE_8):
        CASE_CODE(INVOKE_9):
        CASE_CODE(INVOKE_10):
        CASE_CODE(INVOKE_11):
        CASE_CODE(INVOKE_12):
        CASE_CODE(INVOKE_13):
        CASE_CODE(INVOKE_14):
        CASE_CODE(INVOKE_15):
        CASE_CODE(INVOKE_16):
        CASE_CODE(INVOKE_17):
        CASE_CODE(INVOKE_18):
        CASE_CODE(INVOKE_19):
        CASE_CODE(INVOKE_20):
        CASE_CODE(INVOKE_21):
        CASE_CODE(INVOKE_22):
        CASE_CODE(INVOKE_23):
        CASE_CODE(INVOKE_24):
        CASE_CODE(INVOKE_25):
        CASE_CODE(INVOKE_26):
        CASE_CODE(INVOKE_27):
        CASE_CODE(INVOKE_28):
        CASE_CODE(INVOKE_29):
        CASE_CODE(INVOKE_30):
        CASE_CODE(INVOKE_31): {
            ObjString *method = READ_STRING();
            int argCount = instruction - OP_INVOKE_0;
            if (!invoke(method, argCount)) {
                return INTERPRET_RUNTIME_ERROR;
            }
            frame = &vm.frames[vm.frameCount - 1];
            DISPATCH();
        }

        CASE_CODE(SUPER_0):
        CASE_CODE(SUPER_1):
        CASE_CODE(SUPER_2):
        CASE_CODE(SUPER_3):
        CASE_CODE(SUPER_4):
        CASE_CODE(SUPER_5):
        CASE_CODE(SUPER_6):
        CASE_CODE(SUPER_7):
        CASE_CODE(SUPER_8):
        CASE_CODE(SUPER_9):
        CASE_CODE(SUPER_10):
        CASE_CODE(SUPER_11):
        CASE_CODE(SUPER_12):
        CASE_CODE(SUPER_13):
        CASE_CODE(SUPER_14):
        CASE_CODE(SUPER_15):
        CASE_CODE(SUPER_16):
        CASE_CODE(SUPER_17):
        CASE_CODE(SUPER_18):
        CASE_CODE(SUPER_19):
        CASE_CODE(SUPER_20):
        CASE_CODE(SUPER_21):
        CASE_CODE(SUPER_22):
        CASE_CODE(SUPER_23):
        CASE_CODE(SUPER_24):
        CASE_CODE(SUPER_25):
        CASE_CODE(SUPER_26):
        CASE_CODE(SUPER_27):
        CASE_CODE(SUPER_28):
        CASE_CODE(SUPER_29):
        CASE_CODE(SUPER_30):
        CASE_CODE(SUPER_31): {
            ObjString *method = READ_STRING();
            int argCount = instruction - OP_SUPER_0;
            ObjClass *superclass = AS_CLASS(pop());
            if (!invokeFromClass(superclass, method, argCount)) {
                return INTERPRET_RUNTIME_ERROR;
            }
            frame = &vm.frames[vm.frameCount - 1];
            DISPATCH();
        }

        CASE_CODE(CLOSURE): {
            ObjFunction *function = AS_FUNCTION(READ_CONSTANT());

            // Create the closure and push it on the stack before creating
            // upvalues so that it doesn't get collected.
            ObjClosure *closure = newClosure(function);
            push(OBJ_VAL(closure));

            // Capture upvalues.
            for (int i = 0; i < closure->upvalueCount; i++) {
                uint8_t isLocal = READ_BYTE();
                uint8_t index = READ_BYTE();
                if (isLocal) {
                    // Make an new upvalue to close over the parent's local
                    // variable.
                    closure->upvalues[i] = captureUpvalue(frame->slots + index);
                } else {
                    // Use the same upvalue as the current call frame.
                    closure->upvalues[i] = frame->closure->upvalues[index];
                }
            }

            DISPATCH();
        }

        CASE_CODE(CLOSE_UPVALUE): {
            closeUpvalues(vm.stackTop - 1);
            pop();
            DISPATCH();
        }

        CASE_CODE(RETURN): {
            Value result = pop();

            // Close any upvalues still in scope.
            closeUpvalues(frame->slots);

            vm.frameCount--;

            if (vm.frameCount == vm.currentFrameCount) {
                vm.currentScriptName = vm.scriptName;
                vm.currentFrameCount = -1;
            }

            if (vm.frameCount == 0) return INTERPRET_OK;

            vm.stackTop = frame->slots;
            push(result);

            frame = &vm.frames[vm.frameCount - 1];
            DISPATCH();
        }

        CASE_CODE(CLASS):
            createClass(READ_STRING(), NULL);
            DISPATCH();

        CASE_CODE(SUBCLASS): {
            Value superclass = peek(0);
            if (!IS_CLASS(superclass)) {
                runtimeError("Superclass must be a class.");
                return INTERPRET_RUNTIME_ERROR;
            }

            createClass(READ_STRING(), AS_CLASS(superclass));
            DISPATCH();
        }

        CASE_CODE(METHOD):
            defineMethod(READ_STRING());
            DISPATCH();

        CASE_CODE(OPEN_FILE): {
            Value openType = pop();
            Value fileName = pop();

            if (!IS_STRING(openType)) {
                runtimeError("File open type must be a string");
                return INTERPRET_RUNTIME_ERROR;
            }

            if (!IS_STRING(fileName)) {
                runtimeError("Filename must be a string");
                return INTERPRET_RUNTIME_ERROR;
            }

            ObjString *openTypeString = AS_STRING(openType);
            ObjString *fileNameString = AS_STRING(fileName);

            ObjFile *file = initFile();
            file->file = fopen(fileNameString->chars, openTypeString->chars);
            file->path = fileNameString->chars;
            file->openType = openTypeString->chars;

            if (file->file == NULL) {
                runtimeError("Unable to open file");
                return INTERPRET_RUNTIME_ERROR;
            }

            push(OBJ_VAL(file));
            DISPATCH();
        }
    }
    //}

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP

    return INTERPRET_RUNTIME_ERROR;

}

InterpretResult interpret(const char *source) {
    ObjFunction *function = compile(source);
    if (function == NULL) return INTERPRET_COMPILE_ERROR;
    push(OBJ_VAL(function));
    ObjClosure *closure = newClosure(function);
    pop();
    callValue(OBJ_VAL(closure), 0);
    InterpretResult result = run();
    return result;
}
