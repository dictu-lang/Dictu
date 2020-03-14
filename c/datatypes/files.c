#include "files.h"
#include "../vm.h"
#include "../memory.h"

static bool writeFile(VM *vm, int argCount, bool newLine) {
    if (argCount != 2) {
        runtimeError(vm, "write() takes 2 arguments (%d given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(vm, 0))) {
        runtimeError(vm, "write() argument must be a string");
        return false;
    }

    ObjString *string = AS_STRING(pop(vm));
    ObjFile *file = AS_FILE(pop(vm));

    if (strcmp(file->openType, "r") == 0) {
        runtimeError(vm, "File is not writable!");
        return false;
    }

    int charsWrote = fprintf(file->file, "%s", string->chars);
    if (newLine)
        fprintf(file->file, "\n");

    fflush(file->file);

    push(vm, NUMBER_VAL(charsWrote));

    return true;
}

static bool readFile(VM *vm, int argCount) {
    if (argCount != 1) {
        runtimeError(vm, "read() takes 1 argument (%d given)", argCount);
        return false;
    }

    ObjFile *file = AS_FILE(pop(vm));

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
        runtimeError(vm, "Not enough memory to read \"%s\".\n", file->path);
        return false;
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file->file);
    if (bytesRead < fileSize) {
        free(buffer);
        runtimeError(vm, "Could not read file \"%s\".\n", file->path);
        return false;
    }

    buffer[bytesRead] = '\0';

    push(vm, OBJ_VAL(copyString(vm, buffer, strlen(buffer))));
    free(buffer);
    return true;
}

static bool readLineFile(VM *vm, int argCount) {
    if (argCount != 1) {
        runtimeError(vm, "readLine() takes 1 argument (%d given)", argCount);
        return false;
    }

    char line[4096];

    ObjFile *file = AS_FILE(pop(vm));
    if (fgets(line, 4096, file->file) != NULL) {
        // Remove newline char
        line[strcspn(line, "\n")] = '\0';
        push(vm, OBJ_VAL(copyString(vm, line, strlen(line))));
    } else {
        push(vm, OBJ_VAL(copyString(vm, "", 0)));
    }
    return true;
}

static bool seekFile(VM *vm, int argCount) {
    if (argCount != 2 && argCount != 3) {
        runtimeError(vm, "seek() takes 2 or 3 arguments (%d given)", argCount);
        return false;
    }

    int seekType = SEEK_SET;

    if (argCount == 3) {
        if (!IS_NUMBER(peek(vm, 0)) || !IS_NUMBER(peek(vm, 1))) {
            runtimeError(vm, "seek() arguments must be numbers");
            return false;
        }

        int seekTypeNum = AS_NUMBER(pop(vm));

        switch (seekTypeNum) {
            case 0:
                seekType = SEEK_SET;
                break;
            case 1:
                seekType = SEEK_CUR;
                break;
            case 2:
                seekType = SEEK_END;
                break;
            default:
                seekType = SEEK_SET;
                break;
        }
    }

    if (!IS_NUMBER(peek(vm, 0))) {
        runtimeError(vm, "seek() argument must be a number");
        return false;
    }

    int offset = AS_NUMBER(pop(vm));
    ObjFile *file = AS_FILE(pop(vm));
    fseek(file->file, offset, seekType);

    push(vm, NIL_VAL);
    return true;
}

bool fileMethods(VM *vm, char *method, int argCount) {
    if (strcmp(method, "write") == 0) {
        return writeFile(vm, argCount, false);
    } else if (strcmp(method, "writeLine") == 0) {
        return writeFile(vm, argCount, true);
    } else if (strcmp(method, "read") == 0) {
        return readFile(vm, argCount);
    } else if (strcmp(method, "readLine") == 0) {
        return readLineFile(vm, argCount);
    } else if (strcmp(method, "seek") == 0) {
        return seekFile(vm, argCount);
    }

    return false;
}
