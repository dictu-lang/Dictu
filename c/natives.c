#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "natives.h"
#include "vm.h"

static void defineNative(VM *vm, const char *name, NativeFn function) {
    push(vm, OBJ_VAL(copyString(vm, name, (int) strlen(name))));
    push(vm, OBJ_VAL(newNative(vm, function)));
    tableSet(vm, &vm->globals, AS_STRING(vm->stack[0]), vm->stack[1]);
    pop(vm);
    pop(vm);
}

// Native functions
static Value numberNative(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "number() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "number() only takes a string as an argument");
        return EMPTY_VAL;
    }

    char *numberString = AS_CSTRING(args[0]);
    double number = strtod(numberString, NULL);

    return NUMBER_VAL(number);
}

static Value strNative(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "str() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        char *valueString = valueToString(args[0]);

        ObjString *string = copyString(vm, valueString, strlen(valueString));
        free(valueString);

        return OBJ_VAL(string);
    }

    return args[0];
}

static Value typeNative(VM *vm, int argCount, Value *args) {
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
            case OBJ_NATIVE_CLASS:
            case OBJ_CLASS:
                return OBJ_VAL(copyString(vm, "class", 5));
            case OBJ_TRAIT:
                return OBJ_VAL(copyString(vm, "trait", 5));
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

static Value setNative(VM *vm, int argCount, Value *args) {
    return OBJ_VAL(initSet(vm));
}

static Value lenNative(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "len() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (IS_STRING(args[0])) {
        return NUMBER_VAL(AS_STRING(args[0])->length);
    } else if (IS_LIST(args[0])) {
        return NUMBER_VAL(AS_LIST(args[0])->values.count);
    } else if (IS_DICT(args[0])) {
        return NUMBER_VAL(AS_DICT(args[0])->count);
    } else if (IS_SET(args[0])) {
        return NUMBER_VAL(AS_SET(args[0])->count);
    }

    runtimeError(vm, "Unsupported type passed to len()", argCount);
    return EMPTY_VAL;
}

static Value boolNative(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "bool() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    return BOOL_VAL(!isFalsey(args[0]));
}

static Value inputNative(VM *vm, int argCount, Value *args) {
    if (argCount > 1) {
        runtimeError(vm, "input() takes 1 argument (%d given).", argCount);
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

    uint8_t len_max = 128;
    uint8_t current_size = len_max;

    char *line = malloc(len_max);

    if (line == NULL) {
        runtimeError(vm, "Memory error on input()!");
        return EMPTY_VAL;
    }

    int c = EOF;
    uint8_t i = 0;
    while ((c = getchar()) != '\n' && c != EOF) {
        line[i++] = (char) c;

        if (i == current_size) {
            current_size = i + len_max;
            line = realloc(line, current_size);

            if (line == NULL) {
                printf("Unable to allocate memory\n");
                exit(71);
            }
        }
    }

    line[i] = '\0';

    Value l = OBJ_VAL(copyString(vm, line, strlen(line)));
    free(line);

    return l;
}

static Value printNative(VM *vm, int argCount, Value *args) {
    if (argCount == 0) {
        printf("\n");
        return NIL_VAL;
    }

    for (int i = 0; i < argCount; ++i) {
        Value value = args[i];
        printValue(value);
        printf("\n");
    }

    return NIL_VAL;
}

static Value assertNative(VM *vm, int argCount, Value *args) {
    Value value = args[0];

    if (!IS_BOOL(value)) {
        runtimeError(vm, "assert() only takes a boolean as an argument.", argCount);
        return EMPTY_VAL;
    }

    value = AS_BOOL(value);
    if (!value) {
        runtimeError(vm, "assert() was false!");
        return EMPTY_VAL;
    }

    return NIL_VAL;
}

static Value isDefinedNative(VM *vm, int argCount, Value *args) {
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
       return BOOL_VAL(true);

    return BOOL_VAL(false);
}

// End of natives

void defineAllNatives(VM *vm) {
    char *nativeNames[] = {
            "len",
            "bool",
            "input",
            "number",
            "str",
            "type",
            "set",
            "print",
            "assert",
            "isDefined"
    };

    NativeFn nativeFunctions[] = {
            lenNative,
            boolNative,
            inputNative,
            numberNative,
            strNative,
            typeNative,
            setNative,
            printNative,
            assertNative,
            isDefinedNative
    };


    for (uint8_t i = 0; i < sizeof(nativeNames) / sizeof(nativeNames[0]); ++i) {
        defineNative(vm, nativeNames[i], nativeFunctions[i]);
    }
}
