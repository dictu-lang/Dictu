#include "files.h"
#include "vm.h"
#include "memory.h"

static bool writeFile(int argCount) {
    if (argCount != 2) {
        runtimeError("write() takes 2 arguments (%d given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(0))) {
        runtimeError("write() argument must be a string");
        return false;
    }

    ObjString *string = AS_STRING(pop());
    ObjFile *file = AS_FILE(pop());

    if (strcmp(file->openType, "w") != 0) {
        runtimeError("write() can only be used with the file in write mode");
        return false;
    }

    int charsWrote = fprintf(file->file, "%s", string->chars);
    fflush(file->file);

    push(NUMBER_VAL(charsWrote));

    return true;
}

bool static readFile(int argCount) {
    if (argCount != 1) {
        runtimeError("read() takes 1 argument (%d given)", argCount);
        return false;
    }

    ObjFile *file = AS_FILE(pop());

    size_t currentPosition = ftell(file->file);
    // Calculate file size
    fseek(file->file, 0L, SEEK_END);
    size_t fileSize = ftell(file->file);
    rewind(file->file);

    // Reset cursor position
    if (currentPosition < fileSize) {
        fileSize -= currentPosition;
        fseek(file->file, currentPosition, SEEK_SET);
    }

    char *buffer = (char *) malloc(fileSize + 1);
    if (buffer == NULL) {
        runtimeError("Not enough memory to read \"%s\".\n", file->path);
        return false;
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file->file);
    if (bytesRead < fileSize) {
        runtimeError("Could not read file \"%s\".\n", file->path);
        return false;
    }

    buffer[bytesRead] = '\0';

    push(OBJ_VAL(copyString(buffer, strlen(buffer))));

    return true;
}

bool static seekFile(int argCount) {
    if (argCount != 2) {
        runtimeError("seek() takes 2 arguments (%d given)", argCount);
        return false;
    }

    if (!IS_NUMBER(peek(0))) {
        runtimeError("seek() argument must be a number");
        return false;
    }

    int offset = AS_NUMBER(pop());
    ObjFile *file = AS_FILE(pop());
    fseek(file->file, offset, SEEK_CUR);

    push(NIL_VAL);
    return true;
}

bool fileMethods(char *method, int argCount) {
    if (strcmp(method, "write") == 0) {
        return writeFile(argCount);
    } else if (strcmp(method, "read") == 0) {
        return readFile(argCount);
    } else if (strcmp(method, "seek") == 0) {
        return seekFile(argCount);
    }

    return false;
}
