#include <time.h> 

#include "log.h"
#include "optionals.h"
#include "../vm/vm.h"

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
        runtimeError(vm, "warn() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }
    
    if (argCount != 1) {
        runtimeError(vm, "print() takes 1 argument (%d given)", argCount);
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

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
