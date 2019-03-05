#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#else

#include <unistd.h>

#endif

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
#include "vm.h"
#include "util.h"
#include "collections.h"

VM vm; // [one]

void defineAllNatives();

static void resetStack() {
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
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
            fprintf(stderr, "script: ");
        } else {
            fprintf(stderr, "%s(): ", function->name->chars);
        }

        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        fputs("\n", stderr);
        va_end(args);
    }
}


static void defineNative(const char *name, NativeFn function) {
    push(OBJ_VAL(copyString(name, (int) strlen(name))));
    push(OBJ_VAL(newNative(function)));
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}


static void defineNativeVoid(const char *name, NativeFnVoid function) {
    push(OBJ_VAL(copyString(name, (int) strlen(name))));
    push(OBJ_VAL(newNativeVoid(function)));
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}

void initVM(bool repl) {
    resetStack();
    vm.objects = NULL;
    vm.listObjects = NULL;
    vm.repl = repl;
    vm.bytesAllocated = 0;
    vm.nextGC = 1024 * 1024;
    vm.grayCount = 0;
    vm.grayCapacity = 0;
    vm.grayStack = NULL;
    initTable(&vm.globals);
    initTable(&vm.strings);
    vm.initString = copyString("init", 4);
    defineAllNatives();
}

void freeVM() {
    freeTable(&vm.globals);
    freeTable(&vm.strings);
    vm.initString = NULL;
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
                native(argCount, vm.stackTop - argCount);
                vm.stackTop -= argCount + 1;
                vm.stackCount -= argCount + 1;
                push(NIL_VAL);
                return true;
            }

            case OBJ_NATIVE: {
                NativeFn native = AS_NATIVE(callee);
                Value result = native(argCount, vm.stackTop - argCount);
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

        vm.stackTop[-argCount] = method;
        return callValue(method, argCount);
    }

    if (IS_LIST(receiver)) {
        return listMethods(name->chars, argCount + 1);
    } else if (IS_DICT(receiver)) {
        return dictMethods(name->chars, argCount + 1);
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

static bool isFalsey(Value value) {
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
    ObjString *replVariable = copyString("_", 1);
    tableSet(&vm.globals, replVariable, value);
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

    for (;;) {

#ifdef DEBUG_TRACE_EXECUTION

        printf("          ");
        for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
          printf("[ ");
          printValue(*slot);
          printf(" ]");
        }
        printf("\n");

        disassembleInstruction(&frame->closure->function->chunk,
            (int)(frame->ip - frame->closure->function->chunk.code));
#endif
        uint8_t instruction = READ_BYTE();
        switch (instruction) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }

            case OP_NIL:
                push(NIL_VAL);
                break;

            case OP_TRUE:
                push(BOOL_VAL(true));
                break;

            case OP_FALSE:
                push(BOOL_VAL(false));
                break;

            case OP_POP_REPL: {
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
                break;
            }

            case OP_POP: {
                pop();
                break;
            }

            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(frame->slots[slot]);
                break;
            }

            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = peek(0);
                break;
            }

            case OP_GET_GLOBAL: {
                ObjString *name = READ_STRING();
                Value value;
                if (!tableGet(&vm.globals, name, &value)) {
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;
            }

            case OP_DEFINE_GLOBAL: {
                ObjString *name = READ_STRING();
                tableSet(&vm.globals, name, peek(0));
                pop();
                break;
            }

            case OP_SET_GLOBAL: {
                ObjString *name = READ_STRING();
                if (tableSet(&vm.globals, name, peek(0))) {
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OP_GET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                push(*frame->closure->upvalues[slot]->value);
                break;
            }

            case OP_SET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                *frame->closure->upvalues[slot]->value = peek(0);
                break;
            }

            case OP_GET_PROPERTY: {
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
                    break;
                }

                if (!bindMethod(instance->klass, name)) {
                    return INTERPRET_RUNTIME_ERROR;
                }

                break;
            }

            case OP_SET_PROPERTY: {
                if (!IS_INSTANCE(peek(1))) {
                    runtimeError("Only instances have fields.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjInstance *instance = AS_INSTANCE(peek(1));
                tableSet(&instance->fields, READ_STRING(), peek(0));
                Value value = pop();
                pop();
                push(value);
                break;
            }

            case OP_GET_SUPER: {
                ObjString *name = READ_STRING();
                ObjClass *superclass = AS_CLASS(pop());
                if (!bindMethod(superclass, name)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }

            case OP_GREATER:
                BINARY_OP(BOOL_VAL, >);
                break;

            case OP_LESS:
                BINARY_OP(BOOL_VAL, <);
                break;

            case OP_ADD: {
                if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                    concatenate();
                } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                } else if (IS_LIST(peek(1))) {
                    Value addValue = pop();
                    Value listValue = pop();

                    ObjList *list = AS_LIST(listValue);
                    writeValueArray(&list->values, addValue);

                    push(OBJ_VAL(list));
                } else {
                    runtimeError("Unsupported operand types.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OP_SUBTRACT:
                BINARY_OP(NUMBER_VAL, -);
                break;

            case OP_INCREMENT: {
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number.");
                }

                push(NUMBER_VAL(AS_NUMBER(pop()) + 1));
                break;
            }

            case OP_DECREMENT: {
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number.");
                }

                push(NUMBER_VAL(AS_NUMBER(pop()) - 1));
                break;
            }

            case OP_MULTIPLY:
                BINARY_OP(NUMBER_VAL, *);
                break;

            case OP_DIVIDE:
                BINARY_OP(NUMBER_VAL, /);
                break;

            case OP_MOD: {
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());

                push(NUMBER_VAL(fmod(a, b)));
                break;
            }

            case OP_NOT:
                push(BOOL_VAL(isFalsey(pop())));
                break;

            case OP_NEGATE:
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;

            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }

            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if (isFalsey(peek(0))) frame->ip += offset;
                break;
            }

            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
                break;
            }

            case OP_BREAK: {

                break;
            }

            case OP_IMPORT: {
                ObjString *fileName = AS_STRING(pop());
                char *s = readFile(fileName->chars);

                ObjFunction *function = compile(s);
                if (function == NULL) return INTERPRET_COMPILE_ERROR;
                push(OBJ_VAL(function));
                ObjClosure *closure = newClosure(function);
                pop();

                frame = &vm.frames[vm.frameCount++];
                frame->ip = closure->function->chunk.code;
                frame->closure = closure;
                frame->slots = vm.stackTop - 1;

                break;
            }

            case OP_NEW_LIST: {
                ObjList *list = initList();
                push(OBJ_VAL(list));
                break;
            }

            case OP_ADD_LIST: {
                Value addValue = pop();
                Value listValue = pop();

                ObjList *list = AS_LIST(listValue);
                writeValueArray(&list->values, addValue);

                push(OBJ_VAL(list));
                break;
            }

            case OP_NEW_DICT: {
                ObjDict *dict = initDict();
                push(OBJ_VAL(dict));
                break;
            }

            case OP_ADD_DICT: {
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
                break;
            }

            case OP_SUBSCRIPT: {
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
                    break;
                }

                runtimeError("Array index out of bounds.");
                return INTERPRET_RUNTIME_ERROR;
            }

            case OP_SUBSCRIPT_ASSIGN: {
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
                    break;
                }

                push(NIL_VAL);

                runtimeError("Array index out of bounds.");
                return INTERPRET_RUNTIME_ERROR;
            }

            case OP_SUBSCRIPT_DICT: {
                Value indexValue = pop();
                Value dictValue = pop();

                if (!IS_STRING(indexValue)) {
                    runtimeError("Dictionary key must be a string.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjDict *dict = AS_DICT(dictValue);
                char *key = AS_CSTRING(indexValue);

                push(searchDict(dict, key));

                break;
            }

            case OP_SUBSCRIPT_DICT_ASSIGN: {
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
                break;
            }

            case OP_CALL_0:
            case OP_CALL_1:
            case OP_CALL_2:
            case OP_CALL_3:
            case OP_CALL_4:
            case OP_CALL_5:
            case OP_CALL_6:
            case OP_CALL_7:
            case OP_CALL_8:
            case OP_CALL_9:
            case OP_CALL_10:
            case OP_CALL_11:
            case OP_CALL_12:
            case OP_CALL_13:
            case OP_CALL_14:
            case OP_CALL_15:
            case OP_CALL_16:
            case OP_CALL_17:
            case OP_CALL_18:
            case OP_CALL_19:
            case OP_CALL_20:
            case OP_CALL_21:
            case OP_CALL_22:
            case OP_CALL_23:
            case OP_CALL_24:
            case OP_CALL_25:
            case OP_CALL_26:
            case OP_CALL_27:
            case OP_CALL_28:
            case OP_CALL_29:
            case OP_CALL_30:
            case OP_CALL_31: {
                int argCount = instruction - OP_CALL_0;
                if (!callValue(peek(argCount), argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }

            case OP_INVOKE_0:
            case OP_INVOKE_1:
            case OP_INVOKE_2:
            case OP_INVOKE_3:
            case OP_INVOKE_4:
            case OP_INVOKE_5:
            case OP_INVOKE_6:
            case OP_INVOKE_7:
            case OP_INVOKE_8:
            case OP_INVOKE_9:
            case OP_INVOKE_10:
            case OP_INVOKE_11:
            case OP_INVOKE_12:
            case OP_INVOKE_13:
            case OP_INVOKE_14:
            case OP_INVOKE_15:
            case OP_INVOKE_16:
            case OP_INVOKE_17:
            case OP_INVOKE_18:
            case OP_INVOKE_19:
            case OP_INVOKE_20:
            case OP_INVOKE_21:
            case OP_INVOKE_22:
            case OP_INVOKE_23:
            case OP_INVOKE_24:
            case OP_INVOKE_25:
            case OP_INVOKE_26:
            case OP_INVOKE_27:
            case OP_INVOKE_28:
            case OP_INVOKE_29:
            case OP_INVOKE_30:
            case OP_INVOKE_31: {
                ObjString *method = READ_STRING();
                int argCount = instruction - OP_INVOKE_0;
                if (!invoke(method, argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }

            case OP_SUPER_0:
            case OP_SUPER_1:
            case OP_SUPER_2:
            case OP_SUPER_3:
            case OP_SUPER_4:
            case OP_SUPER_5:
            case OP_SUPER_6:
            case OP_SUPER_7:
            case OP_SUPER_8:
            case OP_SUPER_9:
            case OP_SUPER_10:
            case OP_SUPER_11:
            case OP_SUPER_12:
            case OP_SUPER_13:
            case OP_SUPER_14:
            case OP_SUPER_15:
            case OP_SUPER_16:
            case OP_SUPER_17:
            case OP_SUPER_18:
            case OP_SUPER_19:
            case OP_SUPER_20:
            case OP_SUPER_21:
            case OP_SUPER_22:
            case OP_SUPER_23:
            case OP_SUPER_24:
            case OP_SUPER_25:
            case OP_SUPER_26:
            case OP_SUPER_27:
            case OP_SUPER_28:
            case OP_SUPER_29:
            case OP_SUPER_30:
            case OP_SUPER_31: {
                ObjString *method = READ_STRING();
                int argCount = instruction - OP_SUPER_0;
                ObjClass *superclass = AS_CLASS(pop());
                if (!invokeFromClass(superclass, method, argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }

            case OP_CLOSURE: {
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

                break;
            }

            case OP_CLOSE_UPVALUE:
                closeUpvalues(vm.stackTop - 1);
                pop();
                break;

            case OP_RETURN: {
                Value result = pop();

                // Close any upvalues still in scope.
                closeUpvalues(frame->slots);

                vm.frameCount--;
                if (vm.frameCount == 0) return INTERPRET_OK;

                vm.stackTop = frame->slots;
                push(result);

                frame = &vm.frames[vm.frameCount - 1];
                break;
            }

            case OP_CLASS:
                createClass(READ_STRING(), NULL);
                break;

            case OP_SUBCLASS: {
                Value superclass = peek(0);
                if (!IS_CLASS(superclass)) {
                    runtimeError("Superclass must be a class.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                createClass(READ_STRING(), AS_CLASS(superclass));
                break;
            }

            case OP_METHOD:
                defineMethod(READ_STRING());
                break;
        }
    }

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP

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

// Native functions
static Value timeNative(int argCount, Value *args) {
    return NUMBER_VAL((double) time(NULL));
}

static Value clockNative(int argCount, Value *args) {
    return NUMBER_VAL((double) clock() / CLOCKS_PER_SEC);
}

static Value numberNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("number() takes exactly one argument (%d given).", argCount);
        return NIL_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError("number() only takes a string as an argument");
        return NIL_VAL;
    }

    char *numberString = AS_CSTRING(args[0]);
    double number = strtod(numberString, NULL);

    return NUMBER_VAL(number);
}

static Value strNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("str() takes exactly one argument (%d given).", argCount);
        return NIL_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("str() only takes a number as an argument");
        return NIL_VAL;
    }

    double number = AS_NUMBER(args[0]);

    int numberStringLength = snprintf(NULL, 0, "%.15g", number) + 1;
    char *numberString = malloc(sizeof(char) * numberStringLength);
    snprintf(numberString, numberStringLength, "%.15g", number);

    ObjString *string = copyString(numberString, numberStringLength - 1);
    free(numberString);

    return OBJ_VAL(string);
}

static Value typeNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("str() takes exactly one argument (%d given).", argCount);
        return NIL_VAL;
    }

    if (IS_BOOL(args[0])) {
        return OBJ_VAL(copyString("bool", 4));
    } else if (IS_NIL(args[0])) {
        return OBJ_VAL(copyString("nil", 3));
    } else if (IS_NUMBER(args[0])) {
        return OBJ_VAL(copyString("number", 6));
    } else if (IS_OBJ(args[0])) {
        switch (OBJ_TYPE(args[0])) {
            //TODO: Add more cases for type()
            case OBJ_CLASS:
                return OBJ_VAL(copyString("class", 5));
            case OBJ_FUNCTION:
                return OBJ_VAL(copyString("function", 8));
            case OBJ_STRING:
                return OBJ_VAL(copyString("string", 6));
            default:
                break;
        }
    }

    return OBJ_VAL(copyString("Unknown Type", 12));
}

static Value lenNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("len() takes exactly one argument (%d given).", argCount);
        return NIL_VAL;
    }

    if (IS_STRING(args[0])) {
        return NUMBER_VAL(AS_STRING(args[0])->length);
    } else if (IS_LIST(args[0])) {
        return NUMBER_VAL(AS_LIST(args[0])->values.count);
    }

    runtimeError("len() only takes a string or a list as an argument.", argCount);
    return NIL_VAL;
}

static Value sumNative(int argCount, Value *args) {
    double sum = 0;

    if (argCount == 0) {
        return NUMBER_VAL(0);
    } else if (argCount == 1 && IS_LIST(args[0])) {
        ObjList *list = AS_LIST(args[0]);
        argCount = list->values.count;
        args = list->values.values;
    }

    for (int i = 0; i < argCount; ++i) {
        Value value = args[i];
        if (!IS_NUMBER(value)) {
            runtimeError("A non-number value passed to sum()");
            return NIL_VAL;
        }
        sum = sum + AS_NUMBER(value);
    }

    return NUMBER_VAL(sum);
}

static Value minNative(int argCount, Value *args) {
    double current;

    if (argCount == 0) {
        return NUMBER_VAL(0);
    } else if (argCount == 1 && IS_LIST(args[0])) {
        ObjList *list = AS_LIST(args[0]);
        argCount = list->values.count;
        args = list->values.values;
    }

    double minimum = AS_NUMBER(args[0]);

    for (int i = 1; i < argCount; ++i) {
        Value value = args[i];
        if (!IS_NUMBER(value)) {
            runtimeError("A non-number value passed to min()");
            return NIL_VAL;
        }

        current = AS_NUMBER(value);

        if (minimum > current) {
            minimum = current;
        }
    }

    return NUMBER_VAL(minimum);
}

static Value maxNative(int argCount, Value *args) {
    double current;

    if (argCount == 0) {
        return NUMBER_VAL(0);
    } else if (argCount == 1 && IS_LIST(args[0])) {
        ObjList *list = AS_LIST(args[0]);
        argCount = list->values.count;
        args = list->values.values;
    }

    double maximum = AS_NUMBER(args[0]);

    for (int i = 1; i < argCount; ++i) {
        Value value = args[i];
        if (!IS_NUMBER(value)) {
            runtimeError("A non-number value passed to max()");
            return NIL_VAL;
        }

        current = AS_NUMBER(value);

        if (maximum < current) {
            maximum = current;
        }
    }

    return NUMBER_VAL(maximum);
}

static Value averageNative(int argCount, Value *args) {
    double average = 0;

    if (argCount == 0) {
        return NUMBER_VAL(0);
    } else if (argCount == 1 && IS_LIST(args[0])) {
        ObjList *list = AS_LIST(args[0]);
        argCount = list->values.count;
        args = list->values.values;
    }

    for (int i = 0; i < argCount; ++i) {
        Value value = args[i];
        if (!IS_NUMBER(value)) {
            runtimeError("A non-number value passed to average()");
            return NIL_VAL;
        }
        average = average + AS_NUMBER(value);
    }

    return NUMBER_VAL(average / argCount);
}

static Value floorNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("floor() takes exactly one argument (%d given).", argCount);
        return NIL_VAL;
    }


    return NUMBER_VAL(floor(AS_NUMBER(args[0])));
}

static Value roundNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("round() takes exactly one argument (%d given).", argCount);
        return NIL_VAL;
    }


    return NUMBER_VAL(round(AS_NUMBER(args[0])));
}

static Value ceilNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("ceil() takes exactly one argument (%d given).", argCount);
        return NIL_VAL;
    }


    return NUMBER_VAL(ceil(AS_NUMBER(args[0])));
}

static Value absNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("abs() takes exactly one argument (%d given).", argCount);
        return NIL_VAL;
    }

    double absValue = AS_NUMBER(args[0]);

    if (absValue < 0)
        return NUMBER_VAL(absValue * -1);
    return NUMBER_VAL(absValue);
}


static Value boolNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("bool() takes exactly one argument (%d given).", argCount);
        return NIL_VAL;
    }

    return BOOL_VAL(!isFalsey(args[0]));
}


static Value inputNative(int argCount, Value *args) {
    if (argCount > 1) {
        runtimeError("input() takes at most one argument (%d given).", argCount);
        return NIL_VAL;
    }

    if (argCount != 0) {
        Value prompt = args[0];
        if (!IS_STRING(prompt)) {
            runtimeError("input() only takes a string argument");
            return NIL_VAL;
        }

        printf("%s", AS_CSTRING(prompt));
    }

    uint8_t len_max = 128;
    uint8_t current_size = len_max;

    char *line = malloc(len_max);

    if (line != NULL) {
        int c = EOF;
        uint8_t i = 0;
        while ((c = getchar()) != '\n' && c != EOF) {
            line[i++] = (char) c;

            if (i == current_size) {
                current_size = i + len_max;
                line = realloc(line, current_size);
            }
        }

        line[i] = '\0';

        Value l = OBJ_VAL(copyString(line, strlen(line)));
        free(line);

        return l;
    }


    return NIL_VAL;
}

// Natives no return



static void sleepNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("sleep() takes exactly one argument (%d  given)", argCount);
        return;
    }

    double stopTime = AS_NUMBER(args[0]);

#ifdef _WIN32
    Sleep(stopTime * 1000);
#else
    sleep(stopTime);
#endif
}

static void printNative(int argCount, Value *args) {
    for (int i = 0; i < argCount; ++i) {
        Value value = args[i];

        if (IS_BOOL(value)) {
            printf(AS_BOOL(value) ? "true" : "false");
        } else if (IS_NIL(value)) {
            printf("nil");
        } else if (IS_NUMBER(value)) {
            printf("%.15g", AS_NUMBER(value));
        } else if (IS_OBJ(value)) {
            printObject(value);
        }

        printf("\n");
    }
}

static void assertNative(int argCount, Value *args) {
    Value value = args[0];

    if (!IS_BOOL(value)) {
        runtimeError("assert() only takes a boolean as an argument.", argCount);
        return;
    }

    value = AS_BOOL(value);
    if (!value)
        runtimeError("assert() was false!");
}

// End of natives

void defineAllNatives() {
    char *nativeNames[] = {
            "clock",
            "sum",
            "min",
            "max",
            "average",
            "floor",
            "round",
            "ceil",
            "abs",
            "time",
            "len",
            "bool",
            "input",
            "number",
            "str",
            "type"
    };

    NativeFn nativeFunctions[] = {
            clockNative,
            sumNative,
            minNative,
            maxNative,
            averageNative,
            floorNative,
            roundNative,
            ceilNative,
            absNative,
            timeNative,
            lenNative,
            boolNative,
            inputNative,
            numberNative,
            strNative,
            typeNative
    };

    char *nativeVoidNames[] = {
            "sleep",
            "print",
            "assert"
    };

    NativeFnVoid nativeVoidFunctions[] = {
            sleepNative,
            printNative,
            assertNative
    };


    for (uint8_t i = 0; i < sizeof(nativeNames) / sizeof(nativeNames[0]); ++i) {
        defineNative(nativeNames[i], nativeFunctions[i]);
    }

    for (uint8_t i = 0; i < sizeof(nativeVoidNames) / sizeof(nativeVoidNames[0]); ++i) {
        defineNativeVoid(nativeVoidNames[i], nativeVoidFunctions[i]);
    }
}
