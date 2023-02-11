#include "files.h"
#include "../memory.h"

static Value writeFile(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "write() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "write() argument must be a string");
        return EMPTY_VAL;
    }

    ObjFile *file = AS_FILE(args[0]);
    ObjString *string = AS_STRING(args[1]);

    if (strcmp(file->openType, "r") == 0) {
        runtimeError(vm, "File is not writable!");
        return EMPTY_VAL;
    }

    int charsWrote = fprintf(file->file, "%s", string->chars);
    fflush(file->file);

    return NUMBER_VAL(charsWrote);
}

static Value writeLineFile(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "writeLine() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "writeLine() argument must be a string");
        return EMPTY_VAL;
    }

    ObjFile *file = AS_FILE(args[0]);
    ObjString *string = AS_STRING(args[1]);

    if (strcmp(file->openType, "r") == 0) {
        runtimeError(vm, "File is not writable!");
        return EMPTY_VAL;
    }

    int charsWrote = fprintf(file->file, "%s\n", string->chars);
    fflush(file->file);

    return NUMBER_VAL(charsWrote);
}

static Value readFullFile(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "read() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjFile *file = AS_FILE(args[0]);

    size_t currentPosition = ftell(file->file);
    // Calculate file size
    fseek(file->file, 0L, SEEK_END);
    size_t fileSize = ftell(file->file);
    fseek(file->file, currentPosition, SEEK_SET);

    char *buffer = ALLOCATE(vm, char, fileSize + 1);
    if (buffer == NULL) {
        runtimeError(vm, "Not enough memory to read \"%s\".\n", file->path);
        return EMPTY_VAL;
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file->file);
    if (bytesRead < fileSize && !feof(file->file)) {
        FREE_ARRAY(vm, char, buffer, fileSize + 1);
        runtimeError(vm, "Could not read file \"%s\".\n", file->path);
        return EMPTY_VAL;
    }

    if (bytesRead != fileSize) {
        buffer = SHRINK_ARRAY(vm, buffer, char, fileSize + 1, bytesRead + 1);
    }

    buffer[bytesRead] = '\0';
    return OBJ_VAL(takeString(vm, buffer, bytesRead));
}

static Value readLineFile(DictuVM *vm, int argCount, Value *args) {
    if (argCount > 1) {
        runtimeError(vm, "readLine() takes at most 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    int readLineBufferSize = 4096;

    if (argCount == 1) {
        if (!IS_NUMBER(args[1])) {
            runtimeError(vm, "readLine() argument must be a number");
            return EMPTY_VAL;
        }

        readLineBufferSize = AS_NUMBER(args[1]);
    }

#ifdef _WIN32
    char line[] = ALLOCATE(vm, char, readLineBufferSize);
#else 
    char line[readLineBufferSize]; 
#endif

    ObjFile *file = AS_FILE(args[0]);
    if (fgets(line, readLineBufferSize, file->file) != NULL) {
        int lineLength = strlen(line);
        // Remove newline char
        if (line[lineLength - 1] == '\n') {
            lineLength--;
            line[lineLength] = '\0';
        }
        return OBJ_VAL(copyString(vm, line, lineLength));
    }

#ifdef _WIN32
    FREE(vm, char, line);
#endif 

    return NIL_VAL;
}

static Value seekFile(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1 && argCount != 2) {
        runtimeError(vm, "seek() takes 1 or 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    int seekType = SEEK_SET;

    if (argCount == 2) {
        if (!IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
            runtimeError(vm, "seek() arguments must be numbers");
            return EMPTY_VAL;
        }

        int seekTypeNum = AS_NUMBER(args[2]);

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

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm, "seek() argument must be a number");
        return EMPTY_VAL;
    }

    int offset = AS_NUMBER(args[1]);
    ObjFile *file = AS_FILE(args[0]);

    if (offset != 0 && !strstr(file->openType, "b")) {
        runtimeError(vm, "seek() may not have non-zero offset if file is opened in text mode");
        return EMPTY_VAL;
    }

    fseek(file->file, offset, seekType);

    return NIL_VAL;
}

void declareFileMethods(DictuVM *vm) {
    defineNative(vm, &vm->fileMethods, "write", writeFile);
    defineNative(vm, &vm->fileMethods, "writeLine", writeLineFile);
    defineNative(vm, &vm->fileMethods, "read", readFullFile);
    defineNative(vm, &vm->fileMethods, "readLine", readLineFile);
    defineNative(vm, &vm->fileMethods, "seek", seekFile);
}
