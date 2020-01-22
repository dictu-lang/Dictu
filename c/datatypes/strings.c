#include "strings.h"
#include "../vm.h"
#include "../memory.h"

static bool splitString(int argCount) {
    if (argCount != 2) {
        runtimeError("split() takes 2 arguments (%d  given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(0))) {
        runtimeError("Argument passed to split() must be a string");
        return false;
    }

    // Ensure the GC does *not* run while we are creating the list
    vm.gc = false;

    char *delimiter = AS_CSTRING(pop());
    ObjString *string = AS_STRING(pop());
    char *tmp = malloc(string->length + 1);
    char *tmpFree = tmp;
    memcpy(tmp, string->chars, string->length + 1);
    int delimiterLength = strlen(delimiter);
    char *token;

    ObjList *list = initList();

    do {
        token = strstr(tmp, delimiter);
        if (token)
            *token = '\0';

        ValueArray *array = &list->values;
        ObjString *str = copyString(tmp, strlen(tmp));

        if (array->capacity < array->count + 1) {
            int oldCapacity = array->capacity;
            array->capacity = GROW_CAPACITY(oldCapacity);
            vm.bytesAllocated += sizeof(Value) * (array->capacity) - sizeof(Value) * (oldCapacity);
            array->values = realloc(array->values, sizeof(Value) * (array->capacity));
        }

        array->values[array->count] = OBJ_VAL(str);
        array->count++;

        tmp = token + delimiterLength;
    } while (token != NULL);

    free(tmpFree);
    push(OBJ_VAL(list));
    vm.gc = true;

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
        runtimeError("replace() takes 3 arguments (%d given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(0)) || !IS_STRING(peek(1))) {
        runtimeError("Argument passed to replace() must be a string");
        return false;
    }

    // Pop values off the stack
    char *replace = AS_CSTRING(pop());
    char *to_replace = AS_CSTRING(pop());
    Value stringValue = pop();
    char *string = AS_CSTRING(stringValue);

    int count = 0;
    int len = strlen(to_replace);
    int replaceLen = strlen(replace);
    int stringLen = strlen(string) + 1;

    // Make a copy of the string so we do not modify the original
    char *tmp = malloc(stringLen);
    char *tmpFree = tmp;
    memcpy(tmp, string, stringLen);

    // Count the occurrences of the needle so we can determine the size
    // of the string we need to allocate
    while((tmp = strstr(tmp, to_replace)) != NULL) {
        count++;
        tmp += len;
    }

    // Reset the pointer
    tmp = tmpFree;

    if (count == 0) {
        free(tmpFree);
        push(stringValue);
        return true;
    }

    int length = strlen(tmp) - count * (len - replaceLen) + 1;
    char *pos;
    char *newStr = malloc(sizeof(char) * length);
    int stringLength = 0;

    for (int i = 0; i < count; ++i) {
        pos = strstr(tmp, to_replace);
        if (pos != NULL)
            *pos = '\0';

        int tmpLength = strlen(tmp);
        memcpy(newStr + stringLength, tmp, tmpLength);
        memcpy(newStr + stringLength + tmpLength, replace, replaceLen);
        stringLength += tmpLength + replaceLen;
        tmp = pos + len;
    }

    memcpy(newStr + stringLength, tmp, strlen(tmp));
    ObjString *newString = copyString(newStr, length - 1);
    push(OBJ_VAL(newString));

    free(newStr);
    free(tmpFree);
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
    free(temp);
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
    free(temp);
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

    memcpy(temp, string->chars, length + 1);
    push(OBJ_VAL(copyString(temp, length + 1)));
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

static bool countString(int argCount) {
    if (argCount != 2) {
        runtimeError("count() takes 2 arguments (%d  given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(0))) {
        runtimeError("Argument passed to count() must be a string");
        return false;
    }

    char *needle = AS_CSTRING(pop());
    char *haystack = AS_CSTRING(pop());

    int count = 0;
    while((haystack = strstr(haystack, needle)))
    {
        count++;
        haystack++;
    }

    push(NUMBER_VAL(count));
    return true;
}

static bool formatString(int argCount) {
    if (argCount == 1) {
        runtimeError("format() takes at least 2 arguments (%d given)", argCount);
        return false;
    }

    int length = 0;
    char **replaceStrings = malloc((argCount - 1) * sizeof(char*));

    for (int j = argCount - 2; j >= 0; --j) {
        Value value = pop();
        if (!IS_STRING(value))
            replaceStrings[j] = valueToString(value);
        else {
            ObjString *strObj = AS_STRING(value);
            char *str = malloc(strObj->length + 1);
            memcpy(str, strObj->chars, strObj->length + 1);
            replaceStrings[j] = str;
        }

        length += strlen(replaceStrings[j]);
    }

    char *string = AS_CSTRING(pop());

    int stringLen = strlen(string) + 1;
    char *tmp = malloc(stringLen);
    char *tmpFree = tmp;
    memcpy(tmp, string, stringLen);

    int count = 0;
    while((tmp = strstr(tmp, "{}")))
    {
        count++;
        tmp++;
    }

    tmp = tmpFree;

    if (count != argCount - 1) {
        runtimeError("format() placeholders do not match arguments");

        for (int i = 0; i < argCount - 1; ++i) {
            free(replaceStrings[i]);
        }

        free(tmp);
        free(replaceStrings);
        return false;
    }

    int fullLength = strlen(string) - count * 2 + length + 1;
    char *pos;
    char *newStr = malloc(sizeof(char) * fullLength);
    int stringLength = 0;

    for (int i = 0; i < argCount - 1; ++i) {
        pos = strstr(tmp, "{}");
        if (pos != NULL)
            *pos = '\0';

        int tmpLength = strlen(tmp);
        int replaceLength = strlen(replaceStrings[i]);
        memcpy(newStr + stringLength, tmp, tmpLength);
        memcpy(newStr + stringLength + tmpLength, replaceStrings[i], replaceLength);
        stringLength += tmpLength + replaceLength;
        tmp = pos + 2;
        free(replaceStrings[i]);
    }

    free(replaceStrings);
    memcpy(newStr + stringLength, tmp, strlen(tmp));
    ObjString *newString = copyString(newStr, fullLength - 1);
    push(OBJ_VAL(newString));

    free(newStr);
    free(tmpFree);
    return true;
}

bool stringMethods(char *method, int argCount) {
    if (strcmp(method, "format") == 0) {
        return formatString(argCount);
    } else if (strcmp(method, "split") == 0) {
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
    } else if (strcmp(method, "count") == 0) {
        return countString(argCount);
    }

    runtimeError("String has no method %s()", method);
    return false;
}
