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

    return newResultSuccess(vm, NIL_VAL);
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

    return newResultSuccess(vm, NIL_VAL);
}

void freeLog(DictuVM *vm, ObjAbstract *abstract) {
    Log *log = (Log*)abstract->data;
    if (log->of != NULL) {
        fclose(log->of);
    }
    
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
        return newResultSuccess(vm, OBJ_VAL(log));
    }
    
    log->prefix = strdup(prefix);

    return newResultSuccess(vm, OBJ_VAL(log));
}

static Value logObjPrint(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "print() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    Log *log = AS_LOG(args[0]);

    struct tm *ptm = timeNow();
    char *msg = AS_CSTRING(args[1]);

    if (log->prefix != NULL) {
        fprintf(log->of, "%04d/%02d/%02d %02d:%02d:%02d %s: %s", 
            ptm->tm_year+2000-100, ptm->tm_mon, ptm->tm_mday, 
            ptm->tm_hour, ptm->tm_min, ptm->tm_sec, log->prefix, msg);

        return newResultSuccess(vm, NIL_VAL);
    }

    fprintf(log->of, "%04d/%02d/%02d %02d:%02d:%02d %s", 
        ptm->tm_year+2000-100, ptm->tm_mon, ptm->tm_mday, 
        ptm->tm_hour, ptm->tm_min, ptm->tm_sec, msg);

    return newResultSuccess(vm, NIL_VAL);
}

static Value logObjPrintln(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "println() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    Log *log = AS_LOG(args[0]);

    struct tm *ptm = timeNow();
    char *msg = AS_CSTRING(args[1]);

    if (log->prefix != NULL) {
        fprintf(log->of, "%04d/%02d/%02d %02d:%02d:%02d %s %s\n", 
            ptm->tm_year+2000-100, ptm->tm_mon, ptm->tm_mday, 
            ptm->tm_hour, ptm->tm_min, ptm->tm_sec, log->prefix, msg);

        return newResultSuccess(vm, NIL_VAL);
    }

    fprintf(log->of, "%04d/%02d/%02d %02d:%02d:%02d %s\n", 
        ptm->tm_year+2000-100, ptm->tm_mon, ptm->tm_mday, 
        ptm->tm_hour, ptm->tm_min, ptm->tm_sec, msg);

    return newResultSuccess(vm, NIL_VAL);
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

    return newResultSuccess(vm, NIL_VAL);
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

    return newResultSuccess(vm, NIL_VAL);
}

ObjAbstract* newLogObj(DictuVM *vm) {
    ObjAbstract *abstract = newAbstract(vm, freeLog);
    push(vm, OBJ_VAL(abstract));

    Log *log = ALLOCATE(vm, Log, 1);

    /**
     * Setup Log object methods
     */
    defineNative(vm, &abstract->values, "setPrefix", logObjSetPrefix);

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

    if (args[0] == 0) {
        log->of = stdout;
    } else {
        log->of = (FILE*)args[0];
    }

    abstract->data = log;
    pop(vm);

    return newResultSuccess(vm, OBJ_VAL(abstract));
}

Value createLogModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Log", 3);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

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
