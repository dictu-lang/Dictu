#include <signal.h>
#ifdef _WIN32
#include "windowsapi.h"
#endif

#include "process.h"

#ifdef _WIN32
#define pid_t int
#endif

#ifdef _WIN32
static char* buildArgs(DictuVM *vm, ObjList* list, int *size) {
    // 3 for 1st arg escape + null terminator
    int length = 3;

    for (int i = 0; i < list->values.count; i++) {
        if (!IS_STRING(list->values.values[i])) {
            return NULL;
        }

        // + 1 for space
        length += AS_STRING(list->values.values[i])->length + 1;
    }

    int len = AS_STRING(list->values.values[0])->length;

    char* string = ALLOCATE(vm, char, length);
    memcpy(string, "\"", 1);
    memcpy(string + 1, AS_CSTRING(list->values.values[0]), len);
    memcpy(string + 1 + len, "\"", 1);
    memcpy(string + 2 + len, " ", 1);

    int pointer = 3 + len;
    for (int i = 1; i < list->values.count; i++) {
        len = AS_STRING(list->values.values[i])->length;
        memcpy(string + pointer, AS_CSTRING(list->values.values[i]), len);
        pointer += len;
        memcpy(string + pointer, " ", 1);
        pointer += 1;
    }
    string[pointer] = '\0';

    *size = length;
    return string;
}

static Value execute(DictuVM* vm, ObjList* argList, bool wait) {
    PROCESS_INFORMATION ProcessInfo;

    STARTUPINFO StartupInfo;

    ZeroMemory(&StartupInfo, sizeof(StartupInfo));
    StartupInfo.cb = sizeof StartupInfo;

    int len;
    char* args = buildArgs(vm, argList, &len);

    if (CreateProcess(NULL, args,
        NULL, NULL, TRUE, 0, NULL,
        NULL, &StartupInfo, &ProcessInfo))
    {
        if (wait) {
            WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
        }
        CloseHandle(ProcessInfo.hThread);
        CloseHandle(ProcessInfo.hProcess);

        FREE_ARRAY(vm, char, args, len);
        return newResultSuccess(vm, NIL_VAL);
    }

    return newResultError(vm, "Unable to start process");
}

static Value executeReturnOutput(DictuVM* vm, ObjList* argList) {
    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFO StartupInfo;

    HANDLE childOutRead = NULL;
    HANDLE childOutWrite = NULL;
    SECURITY_ATTRIBUTES saAttr;

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = true;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&childOutRead, &childOutWrite, &saAttr, 0)) {
        return newResultError(vm, "Unable to start process");
    }

    ZeroMemory(&StartupInfo, sizeof(StartupInfo));
    StartupInfo.cb = sizeof StartupInfo;
    StartupInfo.hStdError = childOutWrite;
    StartupInfo.hStdOutput = childOutWrite;
    StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

    int len;
    char* args = buildArgs(vm, argList, &len);

    if (!CreateProcess(NULL, args,
        NULL, NULL, TRUE, 0, NULL,
        NULL, &StartupInfo, &ProcessInfo))
    {
        FREE_ARRAY(vm, char, args, len);
        return newResultError(vm, "Unable to start process");
    }

    WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
    CloseHandle(ProcessInfo.hThread);
    CloseHandle(ProcessInfo.hProcess);
    CloseHandle(childOutWrite);
    FREE_ARRAY(vm, char, args, len);

    DWORD dwRead;
    int size = 1024;
    char* output = ALLOCATE(vm, char, size);
    char buffer[1024];
    int total = 0;

    for (;;) {
        bool ret = ReadFile(childOutRead, buffer, 1024, &dwRead, NULL);

        if (!ret || dwRead == 0)
            break;

        if (total >= size) {
            output = GROW_ARRAY(vm, output, char, size, size * 3);
            size *= 3;
        }

        memcpy(output + total, buffer, dwRead);
        total += dwRead;
    }

    output = SHRINK_ARRAY(vm, output, char, size, total + 1);
    output[total] = '\0';

    return newResultSuccess(vm, OBJ_VAL(takeString(vm, output, total)));
}
#else
static Value execute(DictuVM* vm, ObjList* argList, bool wait) {
    char** arguments = ALLOCATE(vm, char*, argList->values.count + 1);
    for (int i = 0; i < argList->values.count; ++i) {
        if (!IS_STRING(argList->values.values[i])) {
            return newResultError(vm, "Arguments passed must all be strings");
        }

        arguments[i] = AS_CSTRING(argList->values.values[i]);
    }

    arguments[argList->values.count] = NULL;
    pid_t pid = fork();
    if (pid == 0) {
        execvp(arguments[0], arguments);
        exit(errno);
    }

    FREE_ARRAY(vm, char*, arguments, argList->values.count + 1);

    if (wait) {
        int status = 0;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && (status = WEXITSTATUS(status)) != 0) {
            ERROR_RESULT;
        }
    }

    return newResultSuccess(vm, NIL_VAL);
}

static Value executeReturnOutput(DictuVM* vm, ObjList* argList) {
    char** arguments = ALLOCATE(vm, char*, argList->values.count + 1);
    for (int i = 0; i < argList->values.count; ++i) {
        if (!IS_STRING(argList->values.values[i])) {
            return newResultError(vm, "Arguments passed must all be strings");
        }

        arguments[i] = AS_CSTRING(argList->values.values[i]);
    }

    arguments[argList->values.count] = NULL;

    int fd[2];
    if (pipe(fd) != 0) {
        ERROR_RESULT;
    }
    pid_t pid = fork();
    if (pid == 0) {
        close(fd[0]);
        dup2(fd[1], 1);
        dup2(fd[1], 2);
        close(fd[1]);

        execvp(arguments[0], arguments);
        exit(errno);
    }

    FREE_ARRAY(vm, char*, arguments, argList->values.count + 1);

    close(fd[1]);

    int size = 1024;
    char* output = ALLOCATE(vm, char, size);
    char buffer[1024];
    int total = 0;
    int numRead;

    while ((numRead = read(fd[0], buffer, 1024)) != 0) {
        if (total + numRead >= size) {
            output = GROW_ARRAY(vm, output, char, size, size * 3);
            size *= 3;
        }

        memcpy(output + total, buffer, numRead);
        total += numRead;
    }

    close(fd[0]);

    int status = 0;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status) && (status = WEXITSTATUS(status)) != 0) {
        ERROR_RESULT;
    }

    output = SHRINK_ARRAY(vm, output, char, size, total + 1);
    output[total] = '\0';

    return newResultSuccess(vm, OBJ_VAL(takeString(vm, output, total)));
}
#endif

static Value execProcess(DictuVM* vm, int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError(vm, "exec() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_LIST(args[0])) {
        int valLength = 0;
        char *val = valueTypeToString(vm, args[0], &valLength);
        runtimeError(vm, "Argument passed to exec() must be a list, got '%s'.", val);
        FREE_ARRAY(vm, char, val, valLength + 1);
        return EMPTY_VAL;
    }

    ObjList* argList = AS_LIST(args[0]);
    return execute(vm, argList, false);
}

static Value runProcess(DictuVM* vm, int argCount, Value* args) {
    if (argCount != 1 && argCount != 2) {
        runtimeError(vm, "run() takes 1 or 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_LIST(args[0])) {
        int valLength = 0;
        char *val = valueTypeToString(vm, args[0], &valLength);
        runtimeError(vm, "Argument passed to run() must be a list, got '%s'.", val);
        FREE_ARRAY(vm, char, val, valLength + 1);
        return EMPTY_VAL;
    }

    bool getOutput = false;

    if (argCount == 2) {
        if (!IS_BOOL(args[1])) {
            int valLength = 0;
            char *val = valueTypeToString(vm, args[1], &valLength);
            runtimeError(vm, "Optional argument passed to run() must be a boolean, got '%s'.", val);
            FREE_ARRAY(vm, char, val, valLength + 1);
            return EMPTY_VAL;
        }

        getOutput = AS_BOOL(args[1]);
    }

    ObjList* argList = AS_LIST(args[0]);

    if (getOutput) {
        return executeReturnOutput(vm, argList);
    }

    return execute(vm, argList, true);
}

#ifdef _WIN32
static Value killProcess(DictuVM* vm, int argCount, Value* args) {
    if (argCount > 2) {
        runtimeError(vm, "kill() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        int valLength = 0;
        char *val = valueTypeToString(vm, args[0], &valLength);
        runtimeError(vm, "Argument passed to kill() must be a number, got '%s'.", val);
        FREE_ARRAY(vm, char, val, valLength + 1);
        return EMPTY_VAL;
    }

    pid_t pid = (pid_t)AS_NUMBER(args[0]);

    HANDLE handle = OpenProcess(PROCESS_TERMINATE, TRUE, (int)pid);
    if (handle != NULL) {   
        TerminateProcess(handle, 0);
        CloseHandle(handle);
    }

    return newResultSuccess(vm, NIL_VAL);
}
#else
static Value killProcess(DictuVM* vm, int argCount, Value* args) {
    if (argCount > 2) {
        runtimeError(vm, "kill() takes 1 or 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        int valLength = 0;
        char *val = valueTypeToString(vm, args[0], &valLength);
        runtimeError(vm, "First argument passed to kill() must be a number, got '%s'.", val);
        FREE_ARRAY(vm, char, val, valLength + 1);
        return EMPTY_VAL;
    }

    pid_t pid = (pid_t)AS_NUMBER(args[0]);
    int signal = 9;

    if (argCount == 2) {
        if (!IS_NUMBER(args[1])) {
            int valLength = 0;
            char *val = valueTypeToString(vm, args[1], &valLength);
            runtimeError(vm, "Second argument passed to kill() must be a number, got '%s'.", val);
            FREE_ARRAY(vm, char, val, valLength + 1);
            return EMPTY_VAL;
        }

        signal = AS_NUMBER(args[1]);
    }

    if (kill(pid, signal) == -1) {
        ERROR_RESULT;
    }

    return newResultSuccess(vm, NIL_VAL);
}
#endif

Value createProcessModule(DictuVM* vm) {
    ObjString* name = copyString(vm, "Process", 7);
    push(vm, OBJ_VAL(name));
    ObjModule* module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define process methods
     */
    defineNative(vm, &module->values, "exec", execProcess);
    defineNative(vm, &module->values, "run", runProcess);
    defineNative(vm, &module->values, "kill", killProcess);

    /**
     * Define process properties
     */
    defineNativeProperty(vm, &module->values, "SIGINT", NUMBER_VAL(2));
    defineNativeProperty(vm, &module->values, "SIGILL", NUMBER_VAL(4));
    defineNativeProperty(vm, &module->values, "SIGFPE", NUMBER_VAL(8));
    defineNativeProperty(vm, &module->values, "SIGKILL", NUMBER_VAL(9));
    defineNativeProperty(vm, &module->values, "SIGSEGV", NUMBER_VAL(11));
    defineNativeProperty(vm, &module->values, "SIGTERM", NUMBER_VAL(15));

#if defined(__Linux__)
    defineNativeProperty(vm, &module->values, "SIGHUP", NUMBER_VAL(1));
    defineNativeProperty(vm, &module->values, "SIGQUIT", NUMBER_VAL(3));
    defineNativeProperty(vm, &module->values, "SIGABRT", NUMBER_VAL(6));
    defineNativeProperty(vm, &module->values, "SIGTRAP", NUMBER_VAL(5));
    defineNativeProperty(vm, &module->values, "SIGIOT", NUMBER_VAL(6));
    defineNativeProperty(vm, &module->values, "SIGBUS", NUMBER_VAL(7));
    defineNativeProperty(vm, &module->values, "SIGUSR1", NUMBER_VAL(10));
    defineNativeProperty(vm, &module->values, "SIGUSR2", NUMBER_VAL(12));
    defineNativeProperty(vm, &module->values, "SIGPIPE", NUMBER_VAL(13));
    defineNativeProperty(vm, &module->values, "SIGALRM", NUMBER_VAL(14));
    defineNativeProperty(vm, &module->values, "SIGSTKFLT", NUMBER_VAL(16));
    defineNativeProperty(vm, &module->values, "SIGCHLD", NUMBER_VAL(17));
    defineNativeProperty(vm, &module->values, "SIGCONT", NUMBER_VAL(18));
    defineNativeProperty(vm, &module->values, "SIGSTOP", NUMBER_VAL(19));
    defineNativeProperty(vm, &module->values, "SIGTSTP", NUMBER_VAL(20));
    defineNativeProperty(vm, &module->values, "SIGTTIN", NUMBER_VAL(21));
    defineNativeProperty(vm, &module->values, "SIGTTOU", NUMBER_VAL(22));
    defineNativeProperty(vm, &module->values, "SIGURG", NUMBER_VAL(23));
    defineNativeProperty(vm, &module->values, "SIGXCPU", NUMBER_VAL(24));
    defineNativeProperty(vm, &module->values, "SIGXFSZ", NUMBER_VAL(25));
    defineNativeProperty(vm, &module->values, "SIGVTALRM", NUMBER_VAL(26));
    defineNativeProperty(vm, &module->values, "SIGPROF", NUMBER_VAL(27));
    defineNativeProperty(vm, &module->values, "SIGWINCH", NUMBER_VAL(28));
    defineNativeProperty(vm, &module->values, "SIGIO", NUMBER_VAL(29));
    defineNativeProperty(vm, &module->values, "SIGPWR", NUMBER_VAL(30));
    defineNativeProperty(vm, &module->values, "SIGSYS", NUMBER_VAL(31));
    defineNativeProperty(vm, &module->values, "SIGUNUSED", NUMBER_VAL(31));
#elif defined(__FreeBSD__) || defined(__APPLE__)
    defineNativeProperty(vm, &module->values, "SIGHUP", NUMBER_VAL(1));
    defineNativeProperty(vm, &module->values, "SIGQUIT", NUMBER_VAL(3));
    defineNativeProperty(vm, &module->values, "SIGTRAP", NUMBER_VAL(5));
    defineNativeProperty(vm, &module->values, "SIGABRT", NUMBER_VAL(6));
    defineNativeProperty(vm, &module->values, "SIGEMT", NUMBER_VAL(7));
    defineNativeProperty(vm, &module->values, "SIGBUS", NUMBER_VAL(10));
    defineNativeProperty(vm, &module->values, "SIGSYS", NUMBER_VAL(12));
    defineNativeProperty(vm, &module->values, "SIGPIPE", NUMBER_VAL(13));
    defineNativeProperty(vm, &module->values, "SIGALRM", NUMBER_VAL(14));
    defineNativeProperty(vm, &module->values, "SIGURG", NUMBER_VAL(16));
    defineNativeProperty(vm, &module->values, "SIGSTOP", NUMBER_VAL(17));
    defineNativeProperty(vm, &module->values, "SIGTSTP", NUMBER_VAL(18));
    defineNativeProperty(vm, &module->values, "SIGCONT", NUMBER_VAL(19));
    defineNativeProperty(vm, &module->values, "SIGCHLD", NUMBER_VAL(20));
    defineNativeProperty(vm, &module->values, "SIGTTIN", NUMBER_VAL(21));
    defineNativeProperty(vm, &module->values, "SIGTTOU", NUMBER_VAL(22));
    defineNativeProperty(vm, &module->values, "SIGIO", NUMBER_VAL(23));
    defineNativeProperty(vm, &module->values, "SIGXCPU", NUMBER_VAL(24));
    defineNativeProperty(vm, &module->values, "SIGXFSZ", NUMBER_VAL(25));
    defineNativeProperty(vm, &module->values, "SIGVTALRM", NUMBER_VAL(26));
    defineNativeProperty(vm, &module->values, "SIGPROF", NUMBER_VAL(27));
    defineNativeProperty(vm, &module->values, "SIGWINCH", NUMBER_VAL(28));
    defineNativeProperty(vm, &module->values, "SIGINFO", NUMBER_VAL(29));
    defineNativeProperty(vm, &module->values, "SIGUSR1", NUMBER_VAL(30));
    defineNativeProperty(vm, &module->values, "SIGUSR2", NUMBER_VAL(31));
    defineNativeProperty(vm, &module->values, "SIGTHR", NUMBER_VAL(32));
    defineNativeProperty(vm, &module->values, "SIGLIBRT", NUMBER_VAL(33));
#elif defined(_WIN32)
    defineNativeProperty(vm, &module->values, "SIGEXIT", NUMBER_VAL(0));
    defineNativeProperty(vm, &module->values, "SIGABRT", NUMBER_VAL(22));
#endif

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
