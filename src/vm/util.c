#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"
#include "memory.h"

char *readFile(DictuVM *vm, const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char *buffer = ALLOCATE(vm, char, fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }

    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

void defineNative(DictuVM *vm, Table *table, const char *name, NativeFn function) {
    ObjNative *native = newNative(vm, function);
    push(vm, OBJ_VAL(native));
    ObjString *methodName = copyString(vm, name, strlen(name));
    push(vm, OBJ_VAL(methodName));
    tableSet(vm, table, methodName, OBJ_VAL(native));
    pop(vm);
    pop(vm);
}

void defineNativeProperty(DictuVM *vm, Table *table, const char *name, Value value) {
    push(vm, value);
    ObjString *propertyName = copyString(vm, name, strlen(name));
    push(vm, OBJ_VAL(propertyName));
    tableSet(vm, table, propertyName, value);
    pop(vm);
    pop(vm);
}

bool isValidKey(Value value) {
    if (IS_NIL(value) || IS_BOOL(value) || IS_NUMBER(value) ||
    IS_STRING(value)) {
        return true;
    }

    return false;
}

Value boolNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "bool() takes no arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    return BOOL_VAL(!isFalsey(args[0]));
}