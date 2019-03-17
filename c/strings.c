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

static bool findString(int argCount) {
    if (argCount < 2 || argCount > 3) {
        runtimeError("find() takes either 2 or 3 arguments (%d  given)", argCount);
        return false;
    }

    int index = 1;

    if (argCount == 3) {
        if (!IS_NUMBER(peek(0))) {
            runtimeError("Index passed to find() must be a number");
            return false;
        }

        index = AS_NUMBER(pop());
    }

    if (!IS_STRING(peek(0))) {
        runtimeError("Substring passed to find() must be a string");
        return false;
    }

    char *substr = AS_CSTRING(pop());
    char *string = AS_CSTRING(pop());

    int position = 0;

    for (int i = 0; i < index; ++i) {
        char *result = strstr(string, substr);
        if (!result) {
            position = -1;
            break;
        }

        position += (result - string) + (i * strlen(substr));
        string = result + strlen(substr);
    }

    push(NUMBER_VAL(position));
    return true;
}

static bool replaceString(int argCount) {
    if (argCount != 3) {
        runtimeError("replace() takes 3 arguments (%d  given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(0))) {
        runtimeError("Argument passed to replace() must be a string");
        return false;
    }

    if (!IS_STRING(peek(1))) {
        runtimeError("Argument passed to replace() must be a string");
        return false;
    }


    char *replace = AS_CSTRING(pop());
    char *to_replace = AS_CSTRING(pop());
    Value stringValue = pop();
    char *string = AS_CSTRING(stringValue);

    int count = 0;
    const char *tmp = string;
    while((tmp = strstr(tmp, to_replace)) != NULL) {
        count++;
        tmp++;
    }

    if (count == 0) {
        push(stringValue);
        return true;
    }

    int length = strlen(string) - count * (strlen(to_replace) - strlen(replace)) + 1;
    char *pos;
    char *newStr = malloc(sizeof(char) * length);

    for (int i = 0; i < count; ++i) {
        pos = strstr(string, to_replace);
        if (pos != NULL)
            *pos = '\0';

        if (i == 0)
            strncpy(newStr, string, strlen(string));
        else
            strncat(newStr, string, strlen(string));

        strncat(newStr, replace, strlen(replace));
        string = pos + strlen(to_replace);
    }

    strncat(newStr, string, strlen(string));
    ObjString *newString = copyString(newStr, length - 1);
    free(newStr);
    push(OBJ_VAL(newString));
    return true;
}

static bool lowerString(int argCount) {
    if (argCount != 1) {
        runtimeError("lower() takes 1 argument (%d  given)", argCount);
        return false;
    }

    ObjString *string = AS_STRING(pop());

    char *temp = malloc(sizeof(char) * (string->length + 1));

    for(int i = 0; string->chars[i]; i++){
        temp[i] = tolower(string->chars[i]);
    }
    temp[string->length] = '\0';

    push(OBJ_VAL(copyString(temp, string->length)));
    return true;
}

static bool upperString(int argCount) {
    if (argCount != 1) {
        runtimeError("upper() takes 1 argument (%d  given)", argCount);
        return false;
    }

    ObjString *string = AS_STRING(pop());

    char *temp = malloc(sizeof(char) * (string->length + 1));

    for(int i = 0; string->chars[i]; i++){
        temp[i] = toupper(string->chars[i]);
    }
    temp[string->length] = '\0';

    push(OBJ_VAL(copyString(temp, string->length)));
    return true;
}

static bool startsWithString(int argCount) {
    if (argCount != 2) {
        runtimeError("startsWith() takes 2 arguments (%d  given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(0))) {
        runtimeError("Argument passed to startsWith() must be a string");
        return false;
    }

    ObjString *start = AS_STRING(pop());
    char *string = AS_CSTRING(pop());

    push(BOOL_VAL(strncmp(string, start->chars, start->length) == 0));
    return true;
}

static bool endsWithString(int argCount) {
    if (argCount != 2) {
        runtimeError("endsWith() takes 2 arguments (%d  given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(0))) {
        runtimeError("Argument passed to endsWith() must be a string");
        return false;
    }

    ObjString *suffix = AS_STRING(pop());
    ObjString *string = AS_STRING(pop());

    if (string->length < suffix->length) {
        push(FALSE_VAL);
        return true;
    }

    push(BOOL_VAL(strcmp(string->chars + (string->length - suffix->length), suffix->chars) == 0));
    return true;
}

static bool leftStripString(int argCount) {
    if (argCount != 1) {
        runtimeError("leftStrip() takes 1 argument (%d  given)", argCount);
        return false;
    }
    ObjString *string = AS_STRING(pop());

    bool charSeen = false;
    int i, count = 0;

    char *temp = malloc(sizeof(char) * (string->length + 1));

    for (i = 0; i < string->length; ++i) {
        if (!charSeen && isspace(string->chars[i])) {
            count++;
            continue;
        }
        temp[i - count] = string->chars[i];
        charSeen = true;
    }
    temp[i - count] = '\0';
    push(OBJ_VAL(copyString(temp, strlen(temp))));
    free(temp);
    return true;
}

static bool rightStripString(int argCount) {
    if (argCount != 1) {
        runtimeError("rightStrip() takes 1 argument (%d  given)", argCount);
        return false;
    }
    ObjString *string = AS_STRING(pop());

    int length;

    char *temp = malloc(sizeof(char) * (string->length + 1));

    for (length = string->length - 1; length > 0; --length) {
        if (!isspace(string->chars[length])) {
            break;
        }
    }

    strncpy(temp, string->chars, length + 1);
    temp[length + 1] = '\0';
    push(OBJ_VAL(copyString(temp, strlen(temp))));
    free(temp);
    return true;
}

static bool stripString(int argCount) {
    if (argCount != 1) {
        runtimeError("strip() takes 1 argument (%d  given)", argCount);
        return false;
    }

    leftStripString(1);
    rightStripString(1);
    return true;
}

bool stringMethods(char *method, int argCount) {
    if (strcmp(method, "split") == 0) {
        return splitString(argCount);
    } else if (strcmp(method, "contains") == 0) {
        return containsString(argCount);
    } else if (strcmp(method, "find") == 0) {
        return findString(argCount);
    } else if (strcmp(method, "replace") == 0) {
        return replaceString(argCount);
    } else if (strcmp(method, "lower") == 0) {
        return lowerString(argCount);
    } else if (strcmp(method, "upper") == 0) {
        return upperString(argCount);
    } else if (strcmp(method, "startsWith") == 0) {
        return startsWithString(argCount);
    } else if (strcmp(method, "endsWith") == 0) {
        return endsWithString(argCount);
    } else if (strcmp(method, "leftStrip") == 0) {
        return leftStripString(argCount);
    } else if (strcmp(method, "rightStrip") == 0) {
        return rightStripString(argCount);
    } else if (strcmp(method, "strip") == 0) {
        return stripString(argCount);
    }

    runtimeError("String has no method %s()", method);
    return false;
}
