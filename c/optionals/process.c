#include "process.h"

static Value execute(VM *vm, ObjList *argList, bool wait) {
    char **arguments = ALLOCATE(vm, char *, argList->values.count + 1);
    for (int i = 0; i < argList->values.count; ++i) {
        arguments[i] = AS_CSTRING(argList->values.values[i]);
    }

    arguments[argList->values.count] = NULL;
    pid_t pid = fork();
    if (pid == 0) {
        execvp(arguments[0], arguments);
        exit(errno);
    }

    FREE_ARRAY(vm, char *, arguments, argList->values.count + 1);

    if (wait) {
        int status = 0;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            status = WEXITSTATUS(status);
        }

        return NUMBER_VAL(status);
    }

    return NIL_VAL;
}

static Value executeReturnOutput(VM *vm, ObjList *argList) {
    char **arguments = ALLOCATE(vm, char *, argList->values.count + 1);
    for (int i = 0; i < argList->values.count; ++i) {
        arguments[i] = AS_CSTRING(argList->values.values[i]);
    }

    arguments[argList->values.count] = NULL;

    int fd[2];
    pipe(fd);
    pid_t pid = fork();
    if (pid == 0) {
        close(fd[0]);
        dup2(fd[1], 1);
        dup2(fd[1], 2);
        close(fd[1]);

        execvp(arguments[0], arguments);
        exit(errno);
    }

    FREE_ARRAY(vm, char *, arguments, argList->values.count + 1);

    close(fd[1]);

    int size = 1024;
    char *output = ALLOCATE(vm, char, size);
    char buffer[1024];
    int total = 0;
    int numRead;

    while ((numRead = read(fd[0], buffer, 1024)) != 0) {
        if (total >= size) {
            output = GROW_ARRAY(vm, output, char, size, size * 3);
            size *= 3;
        }

        memcpy(output + total, buffer, numRead);
        total += numRead;
    }

    output[total] = '\0';
    vm->bytesAllocated -= size - total - 1;

    return OBJ_VAL(takeString(vm, output, total));
}

static Value execNative(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "exec() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_LIST(args[0])) {
        runtimeError(vm, "Argument passed to exec() must be a string");
        return EMPTY_VAL;
    }

    ObjList *argList = AS_LIST(args[0]);
    return execute(vm, argList, false);
}

static Value runNative(VM *vm, int argCount, Value *args) {
    if (argCount != 1 && argCount != 2) {
        runtimeError(vm, "run() takes 1 or 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_LIST(args[0])) {
        runtimeError(vm, "Argument passed to run() must be a string");
        return EMPTY_VAL;
    }

    bool getOutput = false;

    if (argCount == 2) {
        if (!IS_BOOL(args[1])) {
            runtimeError(vm, "Optional argument passed to run() must be a boolean");
            return EMPTY_VAL;
        }

        getOutput = AS_BOOL(args[1]);
    }

    ObjList *argList = AS_LIST(args[0]);

    if (getOutput) {
        return executeReturnOutput(vm, argList);
    }

    return execute(vm, argList, true);
}

ObjModule *createProcessModule(VM *vm) {
    ObjString *name = copyString(vm, "Process", 7);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define process methods
     */
    defineNative(vm, &module->values, "exec", execNative);
    defineNative(vm, &module->values, "run", runNative);

    /**
     * Define process properties
     */

    pop(vm);
    pop(vm);

    return module;
}
