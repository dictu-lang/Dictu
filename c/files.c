#include "files.h"
#include "vm.h"
#include "memory.h"

static bool writeFile(int argCount, bool newLine) {
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

    if (strcmp(file->openType, "r") == 0) {
        runtimeError("File is not writable!");
        return false;
    }

    int charsWrote = fprintf(file->file, "%s", string->chars);
    if (newLine)
        fprintf(file->file, "\n");

    fflush(file->file);

    push(NUMBER_VAL(charsWrote));

    return true;
}

static bool readFile(int argCount) {
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
    free(buffer);
    return true;
}

static bool readLineFile(int argCount) {
    if (argCount != 1) {
        runtimeError("readLine() takes 1 argument (%d given)", argCount);
        return false;
    }

    char line[4096];

    ObjFile *file = AS_FILE(pop());
    if (fgets(line, 4096, file->file) != NULL) {
        push(OBJ_VAL(copyString(line, strlen(line))));
    } else {
        push(OBJ_VAL(copyString("", 0)));
    }
    return true;
}

static bool seekFile(int argCount) {
    if (argCount < 2 || argCount > 3) {
        runtimeError("seek() takes 2 or 3 arguments (%d given)", argCount);
        return false;
    }

    int seekType = SEEK_SET;

    if (argCount == 3) {
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {
            runtimeError("seek() arguments must be numbers");
            return false;
        }

        int seekTypeNum = AS_NUMBER(pop());

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

    if (!IS_NUMBER(peek(0))) {
        runtimeError("seek() argument must be a number");
        return false;
    }

    int offset = AS_NUMBER(pop());
    ObjFile *file = AS_FILE(pop());
    fseek(file->file, offset, seekType);

    push(NIL_VAL);
    return true;
}

bool fileMethods(char *method, int argCount) {
    if (strcmp(method, "write") == 0) {
        return writeFile(argCount, false);
    } else if (strcmp(method, "writeLine") == 0) {
        return writeFile(argCount, true);
    } else if (strcmp(method, "read") == 0) {
        return readFile(argCount);
    } else if (strcmp(method, "readLine") == 0) {
        return readLineFile(argCount);
    } else if (strcmp(method, "seek") == 0) {
        return seekFile(argCount);
    }

    return false;
}
