#include "strings.h"
#include "../vm.h"
#include "../memory.h"


static Value formatString(VM *vm, int argCount, Value *args) {
    if (argCount == 0) {
        runtimeError(vm, "format() takes at least 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    int length = 0;
    char **replaceStrings = malloc(argCount * sizeof(char*));

    for (int j = 1; j < argCount + 1; j++) {
        Value value = args[j];
        if (!IS_STRING(value))
            replaceStrings[j - 1] = valueToString(value);
        else {
            ObjString *strObj = AS_STRING(value);
            char *str = malloc(strObj->length + 1);
            memcpy(str, strObj->chars, strObj->length + 1);
            replaceStrings[j - 1] = str;
        }

        length += strlen(replaceStrings[j - 1]);
    }

    ObjString *string = AS_STRING(args[0]);

    int stringLen = string->length + 1;
    char *tmp = malloc(stringLen);
    char *tmpFree = tmp;
    memcpy(tmp, string->chars, stringLen);

    int count = 0;
    while((tmp = strstr(tmp, "{}")))
    {
        count++;
        tmp++;
    }

    tmp = tmpFree;

    if (count != argCount) {
        runtimeError(vm, "format() placeholders do not match arguments");

        for (int i = 0; i < argCount; ++i) {
            free(replaceStrings[i]);
        }

        free(tmp);
        free(replaceStrings);
        return EMPTY_VAL;
    }

    int fullLength = string->length - count * 2 + length + 1;
    char *pos;
    char *newStr = malloc(sizeof(char) * fullLength);
    int stringLength = 0;

    for (int i = 0; i < argCount; ++i) {
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
    ObjString *newString = copyString(vm, newStr, fullLength - 1);

    free(newStr);
    free(tmpFree);
    return OBJ_VAL(newString);
}

static Value splitString(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "split() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "Argument passed to split() must be a string");
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);
    char *delimiter = AS_CSTRING(args[1]);

    char *tmp = malloc(string->length + 1);
    char *tmpFree = tmp;
    memcpy(tmp, string->chars, string->length + 1);
    int delimiterLength = strlen(delimiter);
    char *token;

    ObjList *list = initList(vm);
    push(vm, OBJ_VAL(list));

    do {
        token = strstr(tmp, delimiter);
        if (token)
            *token = '\0';

        Value str = OBJ_VAL(copyString(vm, tmp, strlen(tmp)));

        // Push to stack to avoid GC
        push(vm, str);
        writeValueArray(vm, &list->values, str);
        pop(vm);

        tmp = token + delimiterLength;
    } while (token != NULL);
    pop(vm);

    free(tmpFree);
    return OBJ_VAL(list);
}

static Value containsString(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "contains() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "Argument passed to contains() must be a string");
        return EMPTY_VAL;
    }

    char *string = AS_CSTRING(args[0]);
    char *delimiter = AS_CSTRING(args[1]);

    if (!strstr(string, delimiter)) {
        return FALSE_VAL;
    }

    return TRUE_VAL;
}

static Value findString(VM *vm, int argCount, Value *args) {
    if (argCount < 1 || argCount > 2) {
        runtimeError(vm, "find() takes either 1 or 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    int index = 1;

    if (argCount == 2) {
        if (!IS_NUMBER(args[2])) {
            runtimeError(vm, "Index passed to find() must be a number");
            return EMPTY_VAL;
        }

        index = AS_NUMBER(args[2]);
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "Substring passed to find() must be a string");
        return EMPTY_VAL;
    }

    char *substr = AS_CSTRING(args[1]);
    char *string = AS_CSTRING(args[0]);

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

    return NUMBER_VAL(position);
}

static Value replaceString(VM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "replace() takes 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0]) || !IS_STRING(args[1])) {
        runtimeError(vm, "Arguments passed to replace() must be a strings");
        return EMPTY_VAL;
    }

    // Pop values off the stack
    Value stringValue = args[0];
    ObjString *to_replace = AS_STRING(args[1]);
    ObjString *replace = AS_STRING(args[2]);
    char *string = AS_CSTRING(stringValue);

    int count = 0;
    int len = to_replace->length;
    int replaceLen = replace->length;
    int stringLen = strlen(string) + 1;

    // Make a copy of the string so we do not modify the original
    char *tmp = malloc(stringLen);
    char *tmpFree = tmp;
    memcpy(tmp, string, stringLen);

    // Count the occurrences of the needle so we can determine the size
    // of the string we need to allocate
    while((tmp = strstr(tmp, to_replace->chars)) != NULL) {
        count++;
        tmp += len;
    }

    // Reset the pointer
    tmp = tmpFree;

    if (count == 0) {
        free(tmpFree);
        return stringValue;
    }

    int length = strlen(tmp) - count * (len - replaceLen) + 1;
    char *pos;
    char *newStr = malloc(sizeof(char) * length);
    int stringLength = 0;

    for (int i = 0; i < count; ++i) {
        pos = strstr(tmp, to_replace->chars);
        if (pos != NULL)
            *pos = '\0';

        int tmpLength = strlen(tmp);
        memcpy(newStr + stringLength, tmp, tmpLength);
        memcpy(newStr + stringLength + tmpLength, replace->chars, replaceLen);
        stringLength += tmpLength + replaceLen;
        tmp = pos + len;
    }

    memcpy(newStr + stringLength, tmp, strlen(tmp));
    ObjString *newString = copyString(vm, newStr, length - 1);

    free(newStr);
    free(tmpFree);
    return OBJ_VAL(newString);
}

static Value lowerString(VM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "lower() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);

    char *temp = malloc(sizeof(char) * (string->length + 1));

    for(int i = 0; string->chars[i]; i++){
        temp[i] = tolower(string->chars[i]);
    }
    temp[string->length] = '\0';

    Value ret = OBJ_VAL(copyString(vm, temp, string->length));
    free(temp);
    return ret;
}

static Value upperString(VM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "upper() takes no arguments (%d given)", argCount);
        return EMPTY_VAL ;
    }

    ObjString *string = AS_STRING(args[0]);

    char *temp = malloc(sizeof(char) * (string->length + 1));

    for(int i = 0; string->chars[i]; i++){
        temp[i] = toupper(string->chars[i]);
    }
    temp[string->length] = '\0';

    Value ret = OBJ_VAL(copyString(vm, temp, string->length));
    free(temp);
    return ret;
}

static Value startsWithString(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "startsWith() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "Argument passed to startsWith() must be a string");
        return EMPTY_VAL;
    }

    char *string = AS_CSTRING(args[0]);
    ObjString *start = AS_STRING(args[1]);

    return BOOL_VAL(strncmp(string, start->chars, start->length) == 0);
}

static Value endsWithString(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "endsWith() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "Argument passed to endsWith() must be a string");
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);
    ObjString *suffix = AS_STRING(args[1]);

    if (string->length < suffix->length) {
        return FALSE_VAL;
    }

    return BOOL_VAL(strcmp(string->chars + (string->length - suffix->length), suffix->chars) == 0);
}

static Value leftStripString(VM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "leftStrip() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }
    ObjString *string = AS_STRING(args[0]);

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
    Value ret = OBJ_VAL(copyString(vm, temp, i - count));
    free(temp);
    return ret;
}

static Value rightStripString(VM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "rightStrip() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);
    int length;
    char *temp = malloc(sizeof(char) * (string->length + 1));

    for (length = string->length - 1; length > 0; --length) {
        if (!isspace(string->chars[length])) {
            break;
        }
    }

    memcpy(temp, string->chars, length + 1);
    Value ret = OBJ_VAL(copyString(vm, temp, length + 1));
    free(temp);
    return ret;
}

static Value stripString(VM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "strip() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Value string = leftStripString(vm, 0, args);
    return rightStripString(vm, 0, &string);
}

static Value countString(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "count() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "Argument passed to count() must be a string");
        return EMPTY_VAL;
    }

    char *haystack = AS_CSTRING(args[0]);
    char *needle = AS_CSTRING(args[1]);

    int count = 0;
    while((haystack = strstr(haystack, needle)))
    {
        count++;
        haystack++;
    }

    return NUMBER_VAL(count);
}

void declareStringMethods(VM *vm) {
    defineNative(vm, &vm->stringMethods, "format", formatString);
    defineNative(vm, &vm->stringMethods, "split", splitString);
    defineNative(vm, &vm->stringMethods, "contains", containsString);
    defineNative(vm, &vm->stringMethods, "find", findString);
    defineNative(vm, &vm->stringMethods, "replace", replaceString);
    defineNative(vm, &vm->stringMethods, "lower", lowerString);
    defineNative(vm, &vm->stringMethods, "upper", upperString);
    defineNative(vm, &vm->stringMethods, "startsWith", startsWithString);
    defineNative(vm, &vm->stringMethods, "endsWith", endsWithString);
    defineNative(vm, &vm->stringMethods, "leftStrip", leftStripString);
    defineNative(vm, &vm->stringMethods, "rightStrip", rightStripString);
    defineNative(vm, &vm->stringMethods, "strip", stripString);
    defineNative(vm, &vm->stringMethods, "count", countString);
}