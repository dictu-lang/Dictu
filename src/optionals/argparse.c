#include <string.h>

#include "argparse.h"

enum argType {
    stringType,
    numberType,
    boolType,
    listType
};

typedef struct {
    char *flag;
    char *desc;
    bool required;
    char *metavar;
    enum argType type;
} Arg;

typedef struct {
    char *name;
    char *desc;
    char *usage;
    Arg **args;
    int argsCount;
    int argsCapacity;
} Argparser;

#define AS_ARGPARSER(v) ((Argparser*)AS_ABSTRACT(v)->data)
#define ARGS_DEFAULT_COUNT 2
#define ARGS_DEFAULT_INC   2

void freeArgParser(DictuVM *vm, ObjAbstract *abstract) {
    Argparser *argParser = (Argparser*)abstract->data;

    if (argParser->name != NULL) {
        FREE(vm, char, argParser->name);
    }
    if (argParser->desc != NULL) {
        FREE(vm, char, argParser->desc);
    }
    if (argParser->usage != NULL) {
        FREE(vm, char, argParser->usage);
    }

    if (argParser->args != NULL) {
        for (int i = 0; i < argParser->argsCapacity; i++) {
            if (argParser->args[i] != NULL) {
                if (argParser->args[i]->flag != NULL) {
                    FREE(vm, char, argParser->args[i]->flag);
                }
                if (argParser->args[i]->desc != NULL) {
                    FREE(vm, char, argParser->args[i]->desc);
                }
                if (argParser->args[i]->metavar != NULL) {
                    FREE(vm, char, argParser->args[i]->metavar);
                }

                FREE(vm, Arg, argParser->args[i]);
            }
        }

        FREE(vm, Arg, argParser->args);
    }

    FREE(vm, Argparser, abstract->data);
}

char *argParserToString(ObjAbstract *abstract) {
    UNUSED(abstract);

    char *argParseString = malloc(sizeof(char) * 11);
    snprintf(argParseString, 11, "<Argparse>");
    return argParseString;
}

static void addArg(Argparser *argParser, Arg *arg) {
    if (argParser->argsCount == argParser->argsCapacity) {
        argParser->argsCapacity *= ARGS_DEFAULT_INC;
        argParser->args = realloc(argParser->args, argParser->argsCapacity * sizeof(Arg*));
    }

    argParser->args[argParser->argsCount++] = arg;
}

static Value addString(DictuVM *vm, int argCount, Value *args) {
    if (argCount < 3) {
        runtimeError(vm, "addString() takes at least 3 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1]) || !IS_STRING(args[2])) {
        runtimeError(vm, "addString() arguments must be strings");
        return EMPTY_VAL;
    }

    if (!IS_BOOL(args[3])) {
        runtimeError(vm, "addString() required argument must be a bool");
        return EMPTY_VAL;
    }

    if (!IS_EMPTY(args[4]) && IS_NIL(args[4])) {
        if (!IS_STRING(args[4])) {
            runtimeError(vm, "addString() metavar argument must be a string");
            return EMPTY_VAL;
        }
    }

    Arg *arg = ALLOCATE(vm, Arg, 1);

    char *flag = AS_STRING(args[1])->chars;
    arg->flag = ALLOCATE(vm, char, strlen(flag));
    strcpy(arg->flag, flag);
    
    char *desc = AS_STRING(args[2])->chars;
    arg->desc = ALLOCATE(vm, char, strlen(desc));
    strcpy(arg->desc, desc);

    arg->required = AS_BOOL(args[3]);
    arg->type = stringType;

    if (!IS_NIL(args[4])) {
        arg->metavar = ALLOCATE(vm, char, strlen(AS_STRING(args[4])->chars));
        strcpy(arg->metavar, AS_STRING(args[4])->chars);
    }

    Argparser *argParser = AS_ARGPARSER(args[0]);
    addArg(argParser, arg);

    return NIL_VAL;
}

static Value addNumber(DictuVM *vm, int argCount, Value *args) {
    if (argCount < 3) {
        runtimeError(vm, "addNumber() takes at least 3 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1]) || !IS_STRING(args[2])) {
        runtimeError(vm, "addNumber() arguments must be strings");
        return EMPTY_VAL;
    }

    if (!IS_BOOL(args[3])) {
        runtimeError(vm, "addNumber() required argument must be a bool");
        return EMPTY_VAL;
    }
    
    if (!IS_EMPTY(args[4]) && IS_NIL(args[4])) {
        if (!IS_STRING(args[4])) {
            runtimeError(vm, "addNumber() metavar argument must be a string");
            return EMPTY_VAL;
        }
    }

    Arg *arg = ALLOCATE(vm, Arg, 1);

    char *flag = AS_STRING(args[1])->chars;
    arg->flag = ALLOCATE(vm, char, strlen(flag));
    strcpy(arg->flag, flag);
    
    char *desc = AS_STRING(args[2])->chars;
    arg->desc = ALLOCATE(vm, char, strlen(desc));
    strcpy(arg->desc, desc);

    arg->required = AS_BOOL(args[3]);
    arg->type = numberType;

    if (!IS_NIL(args[4])) {
        arg->metavar = ALLOCATE(vm, char, strlen(AS_STRING(args[4])->chars));
        strcpy(arg->metavar, AS_STRING(args[4])->chars);
    }

    Argparser *argParser = AS_ARGPARSER(args[0]);
    addArg(argParser, arg);

    return NIL_VAL;
}

static Value addBool(DictuVM *vm, int argCount, Value *args) {
    if (argCount < 3) {
        runtimeError(vm, "addBool() takes at least 3 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1]) || !IS_STRING(args[2])) {
        runtimeError(vm, "addBool() arguments must be strings");
        return EMPTY_VAL;
    }

    if (!IS_BOOL(args[3])) {
        runtimeError(vm, "addBool() required argument must be a bool");
        return EMPTY_VAL;
    }
    
    if (!IS_EMPTY(args[4]) && IS_NIL(args[4])) {
        if (!IS_STRING(args[4])) {
            runtimeError(vm, "addBool() metavar argument must be a string");
            return EMPTY_VAL;
        }
    }

    Arg *arg = ALLOCATE(vm, Arg, 1);

    char *flag = AS_STRING(args[1])->chars;
    arg->flag = ALLOCATE(vm, char, strlen(flag));
    strcpy(arg->flag, flag);
    
    char *desc = AS_STRING(args[2])->chars;
    arg->desc = ALLOCATE(vm, char, strlen(desc));
    strcpy(arg->desc, desc);

    arg->required = AS_BOOL(args[3]);
    arg->type = boolType;

    if (!IS_NIL(args[4])) {
        arg->metavar = ALLOCATE(vm, char, strlen(AS_STRING(args[4])->chars));
        strcpy(arg->metavar, AS_STRING(args[4])->chars);
    }

    Argparser *argParser = AS_ARGPARSER(args[0]);
    addArg(argParser, arg);

    return NIL_VAL;
}

static Value addList(DictuVM *vm, int argCount, Value *args) {
    if (argCount < 3) {
        runtimeError(vm, "addList() takes at least 3 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1]) || !IS_STRING(args[2])) {
        runtimeError(vm, "addList() arguments must be strings");
        return EMPTY_VAL;
    }

    if (!IS_BOOL(args[3])) {
        runtimeError(vm, "addList() required argument must be a bool");
        return EMPTY_VAL;
    }
    
    if (!IS_EMPTY(args[4]) && IS_NIL(args[4])) {
        if (!IS_STRING(args[4])) {
            runtimeError(vm, "addList() metavar argument must be a string");
            return EMPTY_VAL;
        }
    }

    Arg *arg = ALLOCATE(vm, Arg, 1);

    char *flag = AS_STRING(args[1])->chars;
    arg->flag = ALLOCATE(vm, char, strlen(flag));
    strcpy(arg->flag, flag);
    
    char *desc = AS_STRING(args[2])->chars;
    arg->desc = ALLOCATE(vm, char, strlen(desc));
    strcpy(arg->desc, desc);

    arg->required = AS_BOOL(args[3]);
    arg->type = listType;

    if (!IS_NIL(args[4])) {
        arg->metavar = ALLOCATE(vm, char, strlen(AS_STRING(args[4])->chars));
        strcpy(arg->metavar, AS_STRING(args[4])->chars);
    }

    Argparser *argParser = AS_ARGPARSER(args[0]);
    addArg(argParser, arg);

    return NIL_VAL;
}

static Value parserUsage(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount > 1) {
        runtimeError(vm, "usage() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Argparser *argParser = AS_ARGPARSER(args[0]);
        
    return OBJ_VAL(copyString(vm, argParser->usage, strlen(argParser->usage)));
}

#define IS_SHORT_FLAG(v) v[0] == '-' && v[1] != '-'

inline static void removeLeadingHyphens(const char *str1, char *str2, bool shortFlag) {
    unsigned long i = shortFlag ? 1 : 2;

    for (; i < strlen(str1); i++) {
        str2[i-1] = str1[i];
    }
}

static Value parse(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount > 1) {
        return newResultError(vm, "parse() takes no arguments");
    }

    Argparser *argParser = AS_ARGPARSER(args[0]);

    ObjDict *argsDict = newDict(vm);
    push(vm, OBJ_VAL(argsDict));

    for (int i = 0; i < argParser->argsCount; i++) {
        for (int j = 0; j < vm->argc; j++) { 
            if (strcmp(argParser->args[i]->flag, vm->argv[j]) == 0) {
                if (argParser->args[i]->type == boolType) {
                    ObjString *flagName;
                    if (argParser->args[i]->metavar != NULL) {
                        flagName = copyString(vm, argParser->args[i]->metavar, strlen(argParser->args[i]->metavar));
                    } else {
                        char *formattedFlag = NULL;
                        if (IS_SHORT_FLAG(argParser->args[i]->flag)) {
                            formattedFlag = ALLOCATE(vm, char, strlen(argParser->args[i]->flag)-1);
                            removeLeadingHyphens(argParser->args[i]->flag, formattedFlag, true);
                            flagName = copyString(vm, formattedFlag, strlen(argParser->args[i]->flag));
                        } else {
                            formattedFlag = ALLOCATE(vm , char, strlen(argParser->args[i]->flag)-2);
                            removeLeadingHyphens(argParser->args[i]->flag, formattedFlag, true);
                            flagName = copyString(vm, formattedFlag, strlen(argParser->args[i]->flag));
                        }
                        FREE(vm, char, formattedFlag);
                    }
                    
                    push(vm, OBJ_VAL(flagName));

                    dictSet(vm, argsDict, OBJ_VAL(flagName), BOOL_VAL(true));
                    pop(vm);
                } else if (argParser->args[i]->type == listType) {
                    if (vm->argv[j+1] == NULL || vm->argv[j+1][0] == '-') {
                        runtimeError(vm, "%s requires an argument", argParser->args[i]->flag);
                        return EMPTY_VAL;
                    }
                    
                    ObjString *flagName;
                    if (argParser->args[i]->metavar != NULL) {
                        flagName = copyString(vm, argParser->args[i]->metavar, strlen(argParser->args[i]->metavar));
                    } else {
                        char *formattedFlag = NULL;
                        if (IS_SHORT_FLAG(argParser->args[i]->flag)) {
                            formattedFlag = ALLOCATE(vm, char, strlen(argParser->args[i]->flag)-1);
                            removeLeadingHyphens(argParser->args[i]->flag, formattedFlag, true);
                            flagName = copyString(vm, formattedFlag, strlen(argParser->args[i]->flag));
                        } else {
                            formattedFlag = ALLOCATE(vm , char, strlen(argParser->args[i]->flag)-2);
                            removeLeadingHyphens(argParser->args[i]->flag, formattedFlag, true);
                            flagName = copyString(vm, formattedFlag, strlen(argParser->args[i]->flag));
                        }
                        FREE(vm, char, formattedFlag);
                    }
                    
                    push(vm, OBJ_VAL(flagName));
                    
                    ObjString *flagVal = copyString(vm, vm->argv[j+1], strlen(vm->argv[j+1]));
                    push(vm, OBJ_VAL(flagVal));

                    ObjList *list = newList(vm);
                    push(vm, OBJ_VAL(list));

                    char *p = strtok(vm->argv[j+1], ",");
                    while (p != NULL) {
                        Value str = OBJ_VAL(copyString(vm, p, strlen(p)));
                        push(vm, str);
                        writeValueArray(vm, &list->values, str);
                        p = strtok(NULL, ",");
                    }

                    dictSet(vm, argsDict, OBJ_VAL(flagName), OBJ_VAL(list));
                    pop(vm);
                    pop(vm);
                    pop(vm);
                    pop(vm);
                    pop(vm);
                } else {
                    if (vm->argv[j+1] == NULL || vm->argv[j+1][0] == '-') {
                        runtimeError(vm, "%s requires an argument", argParser->args[i]->flag);
                        return EMPTY_VAL;
                    }
                    
                    ObjString *flagName;
                    if (argParser->args[i]->metavar != NULL) {
                        flagName = copyString(vm, argParser->args[i]->metavar, strlen(argParser->args[i]->metavar));
                    } else {
                        char *formattedFlag = NULL;
                        if (IS_SHORT_FLAG(argParser->args[i]->flag)) {
                            formattedFlag = ALLOCATE(vm, char, strlen(argParser->args[i]->flag)-1);
                            removeLeadingHyphens(argParser->args[i]->flag, formattedFlag, true);
                            flagName = copyString(vm, formattedFlag, strlen(argParser->args[i]->flag));
                        } else {
                            formattedFlag = ALLOCATE(vm , char, strlen(argParser->args[i]->flag)-2);
                            removeLeadingHyphens(argParser->args[i]->flag, formattedFlag, true);
                            flagName = copyString(vm, formattedFlag, strlen(argParser->args[i]->flag));
                        }
                        FREE(vm, char, formattedFlag);
                    }
                    
                    push(vm, OBJ_VAL(flagName));
                    
                    ObjString *flagVal = copyString(vm, vm->argv[j+1], strlen(vm->argv[j+1]));
                    push(vm, OBJ_VAL(flagVal));
                    dictSet(vm, argsDict, OBJ_VAL(flagName), OBJ_VAL(flagVal));
                    pop(vm);
                    pop(vm);
                }
            } 
        }
    }

    pop(vm);
    
    return newResultSuccess(vm, OBJ_VAL(argsDict));
}

ObjAbstract *newParser(DictuVM *vm, char *name, char *desc, char *usage) {
    ObjAbstract *abstract = newAbstract(vm, freeArgParser, argParserToString);
    push(vm, OBJ_VAL(abstract));

    Argparser *argParser = ALLOCATE(vm, Argparser, 1);

    argParser->name = ALLOCATE(vm, char, strlen(name));
    strcpy(argParser->name, name);

    argParser->desc = ALLOCATE(vm, char, strlen(desc));
    strcpy(argParser->desc, desc);

    argParser->usage = ALLOCATE(vm, char, strlen(usage)+1);
    strcpy(argParser->usage, usage);

    argParser->args = ALLOCATE(vm, Arg*, sizeof(Arg) * ARGS_DEFAULT_COUNT);
    argParser->argsCount = 0;
    argParser->argsCapacity = ARGS_DEFAULT_COUNT;

    for (int i = 0; i < ARGS_DEFAULT_COUNT; i++) {
        argParser->args[i] = NULL;
    }

    abstract->data = argParser;

    /**
     * Setup ArgParser object methods
     */
    defineNative(vm, &abstract->values, "addString", addString);
    defineNative(vm, &abstract->values, "addNumber", addNumber);
    defineNative(vm, &abstract->values, "addBool", addBool);
    defineNative(vm, &abstract->values, "addList", addList);
    defineNative(vm, &abstract->values, "parse", parse);
    defineNative(vm, &abstract->values, "usage", parserUsage);
    pop(vm);

    return abstract;
}

static Value createParser(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 3) {
        runtimeError(vm, "newParser() takes 3 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0]) || !IS_STRING(args[1]) || !IS_STRING(args[2])) {
        runtimeError(vm, "newParser() arguments must be strings");
        return EMPTY_VAL;
    }

    char *name = AS_STRING(args[0])->chars;
    if (strlen(name) == 0) {
        runtimeError(vm, "name cannot be empty");
        return EMPTY_VAL;
    }

    char *desc = AS_STRING(args[1])->chars;
    if (strlen(desc) == 0) {
        runtimeError(vm, "description cannot be empty");
        return EMPTY_VAL;
    }

    char *usage = AS_STRING(args[2])->chars;

    ObjAbstract *p = newParser(vm, name, desc, usage);
    return OBJ_VAL(p);
}

Value createArgParseModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Argparse", 8);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Argparse methods
     */
    defineNative(vm, &module->values, "newParser", createParser);

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}