#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "natives.h"
#include "vm.h"

static void defineNative(const char *name, NativeFn function) {
    push(OBJ_VAL(copyString(name, (int) strlen(name))));
    push(OBJ_VAL(newNative(function)));
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}

// Native functions
static Value numberNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("number() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError("number() only takes a string as an argument");
        return EMPTY_VAL;
    }

    char *numberString = AS_CSTRING(args[0]);
    double number = strtod(numberString, NULL);

    return NUMBER_VAL(number);
}

static Value strNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("str() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        char *valueString = valueToString(args[0]);

        ObjString *string = copyString(valueString, strlen(valueString));
        free(valueString);

        return OBJ_VAL(string);
    }

    return args[0];
}

static Value typeNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("type() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (IS_BOOL(args[0])) {
        return OBJ_VAL(copyString("bool", 4));
    } else if (IS_NIL(args[0])) {
        return OBJ_VAL(copyString("nil", 3));
    } else if (IS_NUMBER(args[0])) {
        return OBJ_VAL(copyString("number", 6));
    } else if (IS_OBJ(args[0])) {
        switch (OBJ_TYPE(args[0])) {
            case OBJ_NATIVE_CLASS:
            case OBJ_CLASS:
                return OBJ_VAL(copyString("class", 5));
            case OBJ_TRAIT:
                return OBJ_VAL(copyString("trait", 5));
            case OBJ_INSTANCE: {
                ObjString *className = AS_INSTANCE(args[0])->klass->name;
                return OBJ_VAL(copyString(className->chars, className->length));
            }
            case OBJ_BOUND_METHOD:
                return OBJ_VAL(copyString("method", 6));
            case OBJ_CLOSURE:
            case OBJ_FUNCTION:
                return OBJ_VAL(copyString("function", 8));
            case OBJ_STRING:
                return OBJ_VAL(copyString("string", 6));
            case OBJ_LIST:
                return OBJ_VAL(copyString("list", 4));
            case OBJ_DICT:
                return OBJ_VAL(copyString("dict", 4));
            case OBJ_SET:
                return OBJ_VAL(copyString("set", 3));
            case OBJ_NATIVE:
                return OBJ_VAL(copyString("native", 6));
            case OBJ_FILE:
                return OBJ_VAL(copyString("file", 4));
            default:
                break;
        }
    }

    return OBJ_VAL(copyString("Unknown Type", 12));
}

static Value setNative(int argCount, Value *args) {
    return OBJ_VAL(initSet());
}

static Value lenNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("len() takes 1 argument (%d given).", argCount);
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

    runtimeError("Unsupported type passed to len()", argCount);
    return EMPTY_VAL;
}

static Value boolNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("bool() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    return BOOL_VAL(!isFalsey(args[0]));
}

static Value inputNative(int argCount, Value *args) {
    if (argCount > 1) {
        runtimeError("input() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (argCount != 0) {
        Value prompt = args[0];
        if (!IS_STRING(prompt)) {
            runtimeError("input() only takes a string argument");
            return EMPTY_VAL;
        }

        printf("%s", AS_CSTRING(prompt));
    }

    uint8_t len_max = 128;
    uint8_t current_size = len_max;

    char *line = malloc(len_max);

    if (line == NULL) {
        runtimeError("Memory error on input()!");
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

    Value l = OBJ_VAL(copyString(line, strlen(line)));
    free(line);

    return l;
}

static Value printNative(int argCount, Value *args) {
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

static Value assertNative(int argCount, Value *args) {
    Value value = args[0];

    if (!IS_BOOL(value)) {
        runtimeError("assert() only takes a boolean as an argument.", argCount);
        return EMPTY_VAL;
    }

    value = AS_BOOL(value);
    if (!value) {
        runtimeError("assert() was false!");
        return EMPTY_VAL;
    }

    return NIL_VAL;
}

// End of natives

void defineAllNatives() {
    char *nativeNames[] = {
            "len",
            "bool",
            "input",
            "number",
            "str",
            "type",
            "set",
            "print",
            "assert"
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
            assertNative
    };


    for (uint8_t i = 0; i < sizeof(nativeNames) / sizeof(nativeNames[0]); ++i) {
        defineNative(nativeNames[i], nativeFunctions[i]);
    }
}
