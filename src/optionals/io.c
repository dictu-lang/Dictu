#include <fcntl.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#if defined(__APPLE__) || defined(__FreeBSD__)
#include <copyfile.h>
#elif defined(__linux__)
#include <sys/sendfile.h>
#endif

#include "io.h"


static Value printIO(DictuVM *vm, int argCount, Value *args) {
    if (argCount == 0) {
        runtimeError(vm, "print() takes 1 or more arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    for (int i = 0; i < argCount; ++i) {
        printValue(args[i]);
    }

    return NIL_VAL;
}

static Value printlnIO(DictuVM *vm, int argCount, Value *args) {
    if (argCount == 0) {
        runtimeError(vm, "println() takes 1 or more arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    for (int i = 0; i < argCount; ++i) {
        printValue(args[i]);
        printf("\n");
    }

    return NIL_VAL;
}


static Value printErrIO(DictuVM *vm, int argCount, Value *args) {
    if (argCount == 0) {
        runtimeError(vm, "printErr() takes 1 or more arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    for (int i = 0; i < argCount; ++i) {
        printValueError(args[i]);
    }

    return NIL_VAL;
}

static Value printErrlnIO(DictuVM *vm, int argCount, Value *args) {
    if (argCount == 0) {
        runtimeError(vm, "printErrLn() takes 1 or more arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    for (int i = 0; i < argCount; ++i) {
        printValueError(args[i]);
         fprintf(stderr, "\n");
    }

    return NIL_VAL;
}

#ifdef _WIN32
static Value copyFileIO(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "copyFile() takes 2 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0]) || !IS_STRING(args[1])) {
        runtimeError(vm, "copyFile() arguments must be strings.");
        return EMPTY_VAL;
    }

    char *srcFile = AS_CSTRING(args[0]);
    char *dstFile = AS_CSTRING(args[1]);

    FILE *sf = fopen(srcFile, "r");
    if (sf == NULL) {
        return newResultError(vm, "cannot open src file");
    }

    FILE *df = fopen(dstFile, "w");
    if (df == NULL) {
        fclose(sf);
        return newResultError(vm, "cannot open dst file");
    }

    int buffer = fgetc(sf);
    while (buffer != EOF) {
        fputc(buffer, df);
        buffer = fgetc(sf);
    }

    fclose(sf);
    fclose(df);

    return newResultSuccess(vm, NIL_VAL);
}
#endif

#ifndef _WIN32
static Value copyFileIO(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "copyFile() takes 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0]) || !IS_STRING(args[1])) {
        runtimeError(vm, "copyFile() arguments must be strings.");
        return EMPTY_VAL;
    }

    char *src = AS_CSTRING(args[0]);
    char *dst = AS_CSTRING(args[1]);

    int in = 0;
    int out = 0;

    if ((in = open(src, O_RDONLY)) == -1) {
        return newResultError(vm, "failed to open src file");
    }
    if ((out = creat(dst, 0660)) == -1) {
        close(in);
        return newResultError(vm, "failed to create/open dst file");
    }

#if defined(__APPLE__) || defined(__FreeBSD__)
    fcopyfile(in, out, 0, COPYFILE_ALL);
#elif defined(__linux__)
    off_t bytes = 0;
    struct stat fileinfo = {0};
    fstat(in, &fileinfo);
    sendfile(out, in, &bytes, fileinfo.st_size);
#endif
    close(in);
    close(out);

    return newResultSuccess(vm, NIL_VAL);
}
#endif

Value createIOModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "IO", 2);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    defineNativeProperty(vm, &module->values, "stdin",  NUMBER_VAL(STDIN_FILENO));
    defineNativeProperty(vm, &module->values, "stdout",  NUMBER_VAL(STDOUT_FILENO));
    defineNativeProperty(vm, &module->values, "stderr",  NUMBER_VAL(STDERR_FILENO));
#ifndef _WIN32
    defineNativeProperty(vm, &module->values, "devNull", OBJ_VAL(copyString(vm, "/dev/null", strlen("/dev/null"))));
#else
    defineNativeProperty(vm, &module->values, "devNull", OBJ_VAL(copyString(vm, "\\\\.\\NUL", strlen("\\\\.\\NUL"))));
#endif

    /**
     * Define IO methods
     */
    defineNative(vm, &module->values, "print", printIO);
    defineNative(vm, &module->values, "println", printlnIO);
    defineNative(vm, &module->values, "eprint", printErrIO);
    defineNative(vm, &module->values, "eprintln", printErrlnIO);
    defineNative(vm, &module->values, "copyFile", copyFileIO);

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
