#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "natives.h"
#include "vm.h"

// Native functions
static Value typeNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "type() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (IS_BOOL(args[0])) {
        return OBJ_VAL(copyString(vm, "bool", 4));
    } else if (IS_NIL(args[0])) {
        return OBJ_VAL(copyString(vm, "nil", 3));
    } else if (IS_NUMBER(args[0])) {
        return OBJ_VAL(copyString(vm, "number", 6));
    } else if (IS_OBJ(args[0])) {
        switch (OBJ_TYPE(args[0])) {
            case OBJ_CLASS: {
                switch (AS_CLASS(args[0])->type) {
                    case CLASS_DEFAULT:
                    case CLASS_ABSTRACT: {
                        return OBJ_VAL(copyString(vm, "class", 5));
                    }
                    case CLASS_TRAIT: {
                        return OBJ_VAL(copyString(vm, "trait", 5));
                    }
                }

                break;
            }
            case OBJ_MODULE: {
                return OBJ_VAL(copyString(vm, "module", 6));
            }
            case OBJ_INSTANCE: {
                ObjString *className = AS_INSTANCE(args[0])->klass->name;
                return OBJ_VAL(copyString(vm, className->chars, className->length));
            }
            case OBJ_BOUND_METHOD:
                return OBJ_VAL(copyString(vm, "method", 6));
            case OBJ_CLOSURE:
            case OBJ_FUNCTION:
                return OBJ_VAL(copyString(vm, "function", 8));
            case OBJ_STRING:
                return OBJ_VAL(copyString(vm, "string", 6));
            case OBJ_LIST:
                return OBJ_VAL(copyString(vm, "list", 4));
            case OBJ_DICT:
                return OBJ_VAL(copyString(vm, "dict", 4));
            case OBJ_SET:
                return OBJ_VAL(copyString(vm, "set", 3));
            case OBJ_NATIVE:
                return OBJ_VAL(copyString(vm, "native", 6));
            case OBJ_FILE:
                return OBJ_VAL(copyString(vm, "file", 4));
            default:
                break;
        }
    }

    return OBJ_VAL(copyString(vm, "Unknown Type", 12));
}

static Value setNative(DictuVM *vm, int argCount, Value *args) {
    ObjSet *set = newSet(vm);
    push(vm, OBJ_VAL(set));

    for (int i = 0; i < argCount; i++) {
        setInsert(vm, set, args[i]);
    }
    pop(vm);

    return OBJ_VAL(set);
}

static Value inputNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount > 1) {
        runtimeError(vm, "input() takes either 0 or 1 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (argCount != 0) {
        Value prompt = args[0];
        if (!IS_STRING(prompt)) {
            runtimeError(vm, "input() only takes a string argument");
            return EMPTY_VAL;
        }

        printf("%s", AS_CSTRING(prompt));
    }

    uint64_t currentSize = 128;
    char *line = ALLOCATE(vm, char, currentSize);

    if (line == NULL) {
        runtimeError(vm, "Memory error on input()!");
        return EMPTY_VAL;
    }

    int c = EOF;
    uint64_t length = 0;
    while ((c = getchar()) != '\n' && c != EOF) {
        line[length++] = (char) c;

        if (length + 1 == currentSize) {
            int oldSize = currentSize;
            currentSize = GROW_CAPACITY(currentSize);
            line = GROW_ARRAY(vm, line, char, oldSize, currentSize);

            if (line == NULL) {
                printf("Unable to allocate memory\n");
                exit(71);
            }
        }
    }

    // If length has changed, shrink
    if (length != currentSize) {
        line = SHRINK_ARRAY(vm, line, char, currentSize, length + 1);
    }

    line[length] = '\0';

    return OBJ_VAL(takeString(vm, line, length));
}

static Value printNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(vm);

    if (argCount == 0) {
        printf("\n");
        return NIL_VAL;
    }

    for (int i = 0; i < argCount; ++i) {
        printValue(args[i]);
        printf("\n");
    }

    return NIL_VAL;
}

static Value assertNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "assert() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (isFalsey(args[0])) {
        runtimeError(vm, "assert() was false!");
        return EMPTY_VAL;
    }

    return NIL_VAL;
}

static Value isDefinedNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "isDefined() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "isDefined() only takes a string as an argument");
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);

    Value value;
    if (tableGet(&vm->globals, string, &value))
       return TRUE_VAL;

    return FALSE_VAL;
}

static Value generateSuccessResult(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "Success() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    return newResultSuccess(vm, args[0]);
}

static Value generateErrorResult(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "Error() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "Error() only takes a string as an argument");
        return EMPTY_VAL;
    }

    return OBJ_VAL(newResult(vm, ERR, args[0]));
}

// End of natives

void defineAllNatives(DictuVM *vm) {
    char *nativeNames[] = {
            "input",
            "type",
            "set",
            "print",
            "assert",
            "isDefined",
            "Success",
            "Error"
    };

    NativeFn nativeFunctions[] = {
            inputNative,
            typeNative,
            setNative,
            printNative,
            assertNative,
            isDefinedNative,
            generateSuccessResult,
            generateErrorResult
    };


    for (uint8_t i = 0; i < sizeof(nativeNames) / sizeof(nativeNames[0]); ++i) {
        defineNative(vm, &vm->globals, nativeNames[i], nativeFunctions[i]);
    }
}
