#include <string.h>
#include <time.h>

#include "log.h"
#include "optionals.h"
#include "../vm/vm.h"

typedef struct {
    FILE *of;
    char *prefix;
} Log;

#define AS_LOG(v) ((Log*)AS_ABSTRACT(v)->data)

static struct tm* timeNow() {
    time_t rawtime = time(NULL);
    if (rawtime == -1) {
        return NULL;
    }
    
    struct tm *ptm = localtime(&rawtime);
    if (ptm == NULL) { 
        return NULL;
    }

    return ptm;
}

static Value printLog(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "print() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    struct tm *ptm = timeNow();

    char *msg = AS_CSTRING(args[0]);
    printf("%04d/%02d/%02d %02d:%02d:%02d %s", 
        ptm->tm_year+2000-100, ptm->tm_mon, ptm->tm_mday, 
        ptm->tm_hour, ptm->tm_min, ptm->tm_sec, msg);

    return newResultSuccess(vm, NIL_VAL);
}

static Value printlnLog(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "println() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }
    
    struct tm *ptm = timeNow();

    char *msg = AS_CSTRING(args[0]);
    printf("%04d/%02d/%02d %02d:%02d:%02d %s\n", 
        ptm->tm_year+2000-100, ptm->tm_mon, ptm->tm_mday, 
        ptm->tm_hour, ptm->tm_min, ptm->tm_sec, msg);

    return newResultSuccess(vm, NIL_VAL);
}

static Value fatalLog(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "fatal() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }
    
    struct tm *ptm = timeNow();

    char *msg = AS_CSTRING(args[0]);
    printf("%04d/%02d/%02d %02d:%02d:%02d %s", 
        ptm->tm_year+2000-100, ptm->tm_mon, ptm->tm_mday, 
        ptm->tm_hour, ptm->tm_min, ptm->tm_sec, msg);
    
    exit(1);
}

static Value fatallnLog(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "fatalln() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }
    
    struct tm *ptm = timeNow();

    char *msg = AS_CSTRING(args[0]);
    printf("%04d/%02d/%02d %02d:%02d:%02d %s\n", 
        ptm->tm_year+2000-100, ptm->tm_mon, ptm->tm_mday, 
        ptm->tm_hour, ptm->tm_min, ptm->tm_sec, msg);
    
    exit(1);
}

void freeLog(DictuVM *vm, ObjAbstract *abstract) {
    Log *log = (Log*)abstract->data;
    
    if (log->prefix != NULL) {
        free(log->prefix);
    }

    FREE(vm, Log, abstract->data);
}

static Value logObjSetPrefix(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "setPrefix() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    Log *log = AS_LOG(args[0]);
    char *prefix = AS_CSTRING(args[1]);
    
    if (strlen(prefix) == 0) {
        return NIL_VAL;
    }
    
    log->prefix = strdup(prefix);

    return NIL_VAL;
}

static Value logObjUnsetPrefix(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "unsetPrefix() takes 0 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    Log *log = AS_LOG(args[0]);
    if (log->prefix != NULL) {
        free(log->prefix);
        log->prefix = NULL;
    }

    return NIL_VAL;
}

static Value logObjPrint(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "print() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    Log *log = AS_LOG(args[0]);

    struct tm *ptm = timeNow();
    char *msg = AS_CSTRING(args[1]);

    if (log->prefix != NULL && strlen(log->prefix) != 0) {
        fprintf(log->of, "%04d/%02d/%02d %02d:%02d:%02d %s %s", 
            ptm->tm_year+2000-100, ptm->tm_mon, ptm->tm_mday, 
            ptm->tm_hour, ptm->tm_min, ptm->tm_sec, log->prefix, msg);

        return NIL_VAL;
    }

    fprintf(log->of, "%04d/%02d/%02d %02d:%02d:%02d %s", 
        ptm->tm_year+2000-100, ptm->tm_mon, ptm->tm_mday, 
        ptm->tm_hour, ptm->tm_min, ptm->tm_sec, msg);

    return NIL_VAL;
}

static Value logObjPrintln(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "println() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    Log *log = AS_LOG(args[0]);

    struct tm *ptm = timeNow();
    char *msg = AS_CSTRING(args[1]);

    if (log->prefix != NULL && strlen(log->prefix) != 0) {
        fprintf(log->of, "%04d/%02d/%02d %02d:%02d:%02d %s %s\n", 
            ptm->tm_year+2000-100, ptm->tm_mon, ptm->tm_mday, 
            ptm->tm_hour, ptm->tm_min, ptm->tm_sec, log->prefix, msg);

        return NIL_VAL;
    }

    fprintf(log->of, "%04d/%02d/%02d %02d:%02d:%02d %s\n", 
        ptm->tm_year+2000-100, ptm->tm_mon, ptm->tm_mday, 
        ptm->tm_hour, ptm->tm_min, ptm->tm_sec, msg);

    return NIL_VAL;
}

static Value logObjFatal(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "fatal() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    Log *log = AS_LOG(args[0]);

    struct tm *ptm = timeNow();

    char *msg = AS_CSTRING(args[1]);
    fprintf(log->of, "%04d/%02d/%02d %02d:%02d:%02d %s\n", 
        ptm->tm_year+2000-100, ptm->tm_mon, ptm->tm_mday, 
        ptm->tm_hour, ptm->tm_min, ptm->tm_sec, msg);

    exit(1);
}

static Value logObjFatalln(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "fatalln() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    Log *log = AS_LOG(args[0]);

    struct tm *ptm = timeNow();

    char *msg = AS_CSTRING(args[1]);
    fprintf(log->of, "%04d/%02d/%02d %02d:%02d:%02d %s\n", 
        ptm->tm_year+2000-100, ptm->tm_mon, ptm->tm_mday, 
        ptm->tm_hour, ptm->tm_min, ptm->tm_sec, msg);

    exit(1);
}

ObjAbstract* newLogObj(DictuVM *vm) {
    ObjAbstract *abstract = newAbstract(vm, freeLog);
    push(vm, OBJ_VAL(abstract));

    Log *log = ALLOCATE(vm, Log, 1);
    log->of = NULL;
    log->prefix = NULL;

    /**
     * Setup Log object methods
     */
    defineNative(vm, &abstract->values, "setPrefix", logObjSetPrefix);
    defineNative(vm, &abstract->values, "unsetPrefix", logObjUnsetPrefix);

    defineNative(vm, &abstract->values, "print", logObjPrint);
    defineNative(vm, &abstract->values, "println", logObjPrintln);
    defineNative(vm, &abstract->values, "fatal", logObjFatal);
    defineNative(vm, &abstract->values, "fatalln", logObjFatalln);

    abstract->data = log;
    pop(vm);

    return abstract;
}

static Value newLog(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "new() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    ObjAbstract *abstract = newLogObj(vm);
    Log *log = abstract->data;

    int output = AS_NUMBER(args[0]);
    switch(output) {
        case STDOUT_FILENO:
            log->of = stdout;
            break;
        case STDERR_FILENO:
            log->of = stderr;
            break;
        default:
            return newResultError(vm, "invalid output destination");
    }
    abstract->data = log;

    push(vm, OBJ_VAL(abstract));
    Value success = newResultSuccess(vm, OBJ_VAL(abstract));
    pop(vm);

    return success;
}

Value createLogModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Log", 3);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    defineNativeProperty(vm, &module->values, "stdin",  NUMBER_VAL(STDIN_FILENO));
    defineNativeProperty(vm, &module->values, "stdout",  NUMBER_VAL(STDOUT_FILENO));
    defineNativeProperty(vm, &module->values, "stderr",  NUMBER_VAL(STDERR_FILENO));

    /**
     * Define Log methods
     */
    defineNative(vm, &module->values, "print", printLog);
    defineNative(vm, &module->values, "println", printlnLog);
    defineNative(vm, &module->values, "fatal", fatalLog);
    defineNative(vm, &module->values, "fatalln", fatallnLog);

    defineNative(vm, &module->values, "new", newLog);

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
