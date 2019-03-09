#include "strings.h"
#include "vm.h"
#include "memory.h"

static bool splitString(int argCount) {
    if (argCount != 2) {
        runtimeError("split() takes 2 arguments (%d  given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(0))) {
        runtimeError("Argument passed to split() must be a string");
        return false;
    }

    char *delimiter = AS_CSTRING(pop());
    char *string = AS_CSTRING(pop());
    char *token;

    ObjList *list = initList();

    do {
        token = strstr(string, delimiter);
        if (token)
            *token = '\0';

        writeValueArray(&list->values, OBJ_VAL(copyString(string, strlen(string))));
        string = token + strlen(delimiter);
    } while(token != NULL);

    push(OBJ_VAL(list));

    return true;
}

static bool containsString(int argCount) {
    if (argCount != 2) {
        runtimeError("contains() takes 2 arguments (%d  given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(0))) {
        runtimeError("Argument passed to contains() must be a string");
        return false;
    }

    char *delimiter = AS_CSTRING(pop());
    char *string = AS_CSTRING(pop());

    if (!strstr(string, delimiter)) {
        push(FALSE_VAL);
        return true;
    }

    push(TRUE_VAL);
    return true;
}

bool stringMethods(char *method, int argCount) {
    if (strcmp(method, "split") == 0) {
        return splitString(argCount);
    } else if (strcmp(method, "contains") == 0) {
        return containsString(argCount);
    }

    runtimeError("String has no method %s()", method);
    return false;
}
