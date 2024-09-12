#include "strings.h"
#include "../memory.h"


static Value lenString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "len() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);
    return NUMBER_VAL(string->length);
}

static Value toNumberString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "toNumber() takes no arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    char *numberString = AS_CSTRING(args[0]);
    char *end;
    errno = 0;

    double number = strtod(numberString, &end);

    // Failed conversion
    if (errno != 0 || *end != '\0') {
        int length = AS_STRING(args[0])->length;

        char *errorMsg = ALLOCATE(vm, char, 29 + length);
        memcpy(errorMsg, "Can not convert '", 17);
        memcpy(errorMsg + 17, numberString, length);
        memcpy(errorMsg + 17 + length, "' to number", 11);
        errorMsg[28 + length] = '\0';

        Value result = newResultError(vm, errorMsg);

        FREE_ARRAY(vm, char, errorMsg, 29 + length);

        return result;
    }

    return newResultSuccess(vm, NUMBER_VAL(number));
}

static Value formatString(DictuVM *vm, int argCount, Value *args) {
    if (argCount == 0) {
        runtimeError(vm, "format() takes at least 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    int length = 0;
    char **replaceStrings = ALLOCATE(vm, char*, argCount);

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
    char *tmp = ALLOCATE(vm, char, stringLen);
    char *tmpFree = tmp;
    memcpy(tmp, string->chars, stringLen);

    int count = 0;
    while ((tmp = strstr(tmp, "{}"))) {
        count++;
        tmp++;
    }

    tmp = tmpFree;

    if (count != argCount) {
        runtimeError(vm, "format() placeholders do not match arguments");

        for (int i = 0; i < argCount; ++i) {
            free(replaceStrings[i]);
        }

        FREE_ARRAY(vm, char, tmp , stringLen);
        FREE_ARRAY(vm, char*, replaceStrings, argCount);
        return EMPTY_VAL;
    }

    int fullLength = string->length - count * 2 + length + 1;
    char *pos;
    char *newStr = ALLOCATE(vm, char, fullLength);
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

    FREE_ARRAY(vm, char*, replaceStrings, argCount);
    memcpy(newStr + stringLength, tmp, strlen(tmp));
    newStr[fullLength - 1] = '\0';
    FREE_ARRAY(vm, char, tmpFree, stringLen);

    return OBJ_VAL(takeString(vm, newStr, fullLength - 1));
}

static Value splitString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1 && argCount != 2) {
        runtimeError(vm, "split() takes 1 or 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "Argument passed to split() must be a string");
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);
    char *delimiter = AS_CSTRING(args[1]);
    int maxSplit = string->length + 1;

    if (argCount == 2) {
        if (!AS_NUMBER(args[1])) {
            runtimeError(vm, "Argument passed to split() must be a number");
            return EMPTY_VAL;
        }

        if (AS_NUMBER(args[2]) >= 0) {
            maxSplit = AS_NUMBER(args[2]);
        }
    }

    char *tmp = ALLOCATE(vm, char, string->length + 1);
    char *tmpFree = tmp;
    memcpy(tmp, string->chars, string->length);
    tmp[string->length] = '\0';
    int delimiterLength = strlen(delimiter);
    char *token;

    ObjList *list = newList(vm);
    push(vm, OBJ_VAL(list));
    int count = 0;

    if (delimiterLength == 0) {
        int tokenIndex = 0;
        for (; tokenIndex < string->length && count < maxSplit; tokenIndex++) {
            count++;
            *(tmp) = string->chars[tokenIndex];
            *(tmp + 1) = '\0';
            Value str = OBJ_VAL(copyString(vm, tmp, 1));
            // Push to stack to avoid GC
            push(vm, str);
            writeValueArray(vm, &list->values, str);
            pop(vm);
        }

        if (tokenIndex != string->length && count >= maxSplit) {
            tmp = (string->chars) + tokenIndex;
        } else {
            tmp = NULL;
        }
    } else if (maxSplit > 0) {
        do {
            count++;
            token = strstr(tmp, delimiter);
            if (token)
                *token = '\0';

            Value str = OBJ_VAL(copyString(vm, tmp, strlen(tmp)));

            // Push to stack to avoid GC
            push(vm, str);
            writeValueArray(vm, &list->values, str);
            pop(vm);

            tmp = token + delimiterLength;
        } while (token != NULL && count < maxSplit);

        if (token == NULL) {
            tmp = NULL;
        }
    }

    if (tmp != NULL && count >= maxSplit) {
        Value remainingStr = OBJ_VAL(copyString(vm, tmp, strlen(tmp)));

        // Push to stack to avoid GC
        push(vm, remainingStr);
        writeValueArray(vm, &list->values, remainingStr);
        pop(vm);
    }

    pop(vm);

    FREE_ARRAY(vm, char, tmpFree, string->length + 1);
    return OBJ_VAL(list);
}

static Value containsString(DictuVM *vm, int argCount, Value *args) {
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

static Value findString(DictuVM *vm, int argCount, Value *args) {
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

static Value findLastString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "findLast() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    const char *str = AS_CSTRING(args[0]);
    const char *ss = AS_CSTRING(args[1]);
    const char *p = str;
    int found = !*ss;

    if (!found) {
        while (*p) {
            ++p;
        }

        const char *q = ss;
        while (*q) {
            ++q;
        }

        while (!found && !(p-str < q-ss)) {
            const char *s = p;
            const char *t = q;

            while (t != ss && *(s-1) == *(t-1)) {
                --s;
                --t;
            }

            found = t == ss;

            if (found) {
                p = s;
            } else {
                --p;
            }
        }
    }

    return NUMBER_VAL(found ? p-str : -1);
}

static Value replaceString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "replace() takes 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1]) || !IS_STRING(args[2])) {
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
    char *tmp = ALLOCATE(vm, char, stringLen + 1);
    char *tmpFree = tmp;
    memcpy(tmp, string, stringLen);
    tmp[stringLen] = '\0';

    // Count the occurrences of the needle so we can determine the size
    // of the string we need to allocate
    while ((tmp = strstr(tmp, to_replace->chars)) != NULL) {
        count++;
        tmp += len;
    }

    // Reset the pointer
    tmp = tmpFree;

    if (count == 0) {
        FREE_ARRAY(vm, char, tmpFree, stringLen + 1);
        return stringValue;
    }

    int length = strlen(tmp) - count * (len - replaceLen) + 1;
    char *pos;
    char *newStr = ALLOCATE(vm, char, length);
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
    FREE_ARRAY(vm, char, tmpFree, stringLen + 1);
    newStr[length - 1] = '\0';

    return OBJ_VAL(takeString(vm, newStr, length - 1));
}

static Value lowerString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "lower() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);
    char *temp = ALLOCATE(vm, char, string->length + 1);

    for (int i = 0; string->chars[i]; i++) {
        temp[i] = tolower(string->chars[i]);
    }
    temp[string->length] = '\0';

    return OBJ_VAL(takeString(vm, temp, string->length));
}

static Value upperString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "upper() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);
    char *temp = ALLOCATE(vm, char, string->length + 1);

    for (int i = 0; string->chars[i]; i++) {
        temp[i] = toupper(string->chars[i]);
    }
    temp[string->length] = '\0';

    return OBJ_VAL(takeString(vm, temp, string->length));
}

static Value startsWithString(DictuVM *vm, int argCount, Value *args) {
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

static Value endsWithString(DictuVM *vm, int argCount, Value *args) {
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

static Value leftStripString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "leftStrip() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);
    int i, count = 0;
    char *temp = ALLOCATE(vm, char, string->length + 1);

    for (i = 0; i < string->length; ++i) {
        if (!isspace(string->chars[i])) {
            break;
        }
        count++;
    }

    if (count != 0) {
        temp = SHRINK_ARRAY(vm, temp, char, string->length + 1, (string->length - count) + 1);
    }

    memcpy(temp, string->chars + count, string->length - count);
    temp[string->length - count] = '\0';
    return OBJ_VAL(takeString(vm, temp, string->length - count));
}

static Value rightStripString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "rightStrip() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);
    int length;
    char *temp = ALLOCATE(vm, char, string->length + 1);

    for (length = string->length - 1; length > 0; --length) {
        if (!isspace(string->chars[length])) {
            break;
        }
    }

    // If characters were stripped resize the buffer
    if (length + 1 != string->length) {
        temp = SHRINK_ARRAY(vm, temp, char, string->length + 1, length + 2);
    }

    memcpy(temp, string->chars, length + 1);
    temp[length + 1] = '\0';
    return OBJ_VAL(takeString(vm, temp, length + 1));
}

static Value stripString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "strip() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Value string = leftStripString(vm, 0, args);
    push(vm, string);
    string = rightStripString(vm, 0, &string);
    pop(vm);
    return string;
}

static Value countString(DictuVM *vm, int argCount, Value *args) {
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
    while ((haystack = strstr(haystack, needle))) {
        count++;
        haystack++;
    }

    return NUMBER_VAL(count);
}

static Value wordCountString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "count() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    char *string = AS_CSTRING(args[0]);
    
    int count = 0;
    int len = strlen(string);
    bool in = false;

    for (int i = 0; i < len; i++) {
        if (isspace(string[i])) {
            in = false;
        } else if(isalpha(string[i])) {
            if(!in) {
                in = true;
                count++;
            }
        }
    }

    return NUMBER_VAL(count);
}

static Value titleString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "title() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);
    char *temp = ALLOCATE(vm, char, string->length + 1);

    bool convertNext=true;

    for (int i = 0; string->chars[i]; i++) {
        if(string->chars[i]==' '){
            convertNext=true;
        }
        else if(convertNext){
            temp[i] = toupper(string->chars[i]);
            convertNext=false;
            continue;
        }
        temp[i] = tolower(string->chars[i]);
    }

    temp[string->length] = '\0';

    return OBJ_VAL(takeString(vm, temp, string->length));
}

static Value repeatString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "repeat() takes one argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm, "repeat() count argument must be a number");
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);
    int count = AS_NUMBER(args[1]);

    int tempLen = (string->length * count) + 1;
    char *temp = ALLOCATE(vm, char, tempLen);

    strcpy(temp, string->chars);
    while (--count > 0) {
        strcat(temp, string->chars);
    }

    return OBJ_VAL(takeString(vm, temp, tempLen - 1));
}

static Value isUpperString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "isUpper() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    char *string = AS_CSTRING(args[0]);
    int len = strlen(string);

    if (len == 0) {
        return BOOL_VAL(false);
    }

    for (int i = 0; i < len; i++) {
        if (!isupper(string[i]) && isalpha(string[i])) {
            return BOOL_VAL(false);
        }
    }
    
    return BOOL_VAL(true);
}

static Value isLowerString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "isLower() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    char *string = AS_CSTRING(args[0]);
    int len = strlen(string);

    if (len == 0) {
        return BOOL_VAL(false);
    }

    for (int i = 0; i < len; i++) {
        if (!islower(string[i]) && isalpha(string[i])) {
            return BOOL_VAL(false);
        }
    }
    
    return BOOL_VAL(true);
}

static Value collapseSpacesString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "collapseSpaces() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);
    char *temp = ALLOCATE(vm, char, string->length + 1);
    strcpy(temp, string->chars);

    int i, j;
    for (i = j = 0; temp[i]; ++i) {
        if (!isspace(temp[i]) || (i > 0 && !isspace(temp[i-1]))) {
            temp[j++] = temp[i];
        }
    }

    temp[j+1] = '\0';

    if (i != j) {
        temp = SHRINK_ARRAY(vm, temp, char, string->length + 1, j + 1);
    }

    return OBJ_VAL(takeString(vm, temp, j));
}

static Value wrapString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "wrap() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);
    char *temp = ALLOCATE(vm, char, string->length + 1);
    
    int len = AS_NUMBER(args[1]);

    int last = 0;
    int count = 0;

    for (int cur = 0; string->chars[cur] != '\0'; cur++, count++) {
        temp[cur] = string->chars[cur];

        if (isspace(temp[cur])) {
            last = cur;
        } 

        if (count >= len) {
            temp[last] = '\n';
            count = 0;
        }
    }

    return OBJ_VAL(takeString(vm, temp, strlen(temp)));
}

static Value snakeToCamelCaseString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "snakeToCamelCase() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);

    if (string->length < 3) {
        return OBJ_VAL(string);
    }
    if (strchr(string->chars, '_') == NULL) {
        return OBJ_VAL(string);
    }

    char *temp = ALLOCATE(vm, char, string->length + 1);
    
    int i, j;
    for (i = j = 0; i < string->length; i++, j++) {
        if (string->chars[i] == '_') {
            temp[j] = toupper(string->chars[i+1]);
            i++;
        } else {
            temp[j] = string->chars[i];
        }
    }
    temp[j+1] = '\0';

    if (i != j) {
        temp = SHRINK_ARRAY(vm, temp, char, string->length + 1, strlen(temp) + 1);
    }

    return OBJ_VAL(takeString(vm, temp, j));
}

static Value camelToSnakeCaseString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "camelToSnakeCase() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);

    if (string->length < 2) {
        return OBJ_VAL(string);
    }

    bool upperCaseFound = false;

    int i, j;
    for (i = 0; i < string->length; i++) {
        if (isupper(string->chars[i])) {
            upperCaseFound = true;
            break;
        }
    }

    if (!upperCaseFound) {
        return OBJ_VAL(string);
    }

    char *temp = ALLOCATE(vm, char, (string->length * 2) + 1);
    
    for (i = j = 0; i < string->length; i++, j++) {
        if (isupper(string->chars[i])) {
            temp[j] = '_';
            j++;
            temp[j] = tolower(string->chars[i]);
        } else {
            temp[j] = string->chars[i];
        }
    }
    temp[j+1] = '\0';

    if (i != j) {
        temp = SHRINK_ARRAY(vm, temp, char, (string->length * 2) + 2, j + 1);
    }

    return OBJ_VAL(takeString(vm, temp, j));
}

void declareStringMethods(DictuVM *vm) {
    defineNative(vm, &vm->stringMethods, "len", lenString);
    defineNative(vm, &vm->stringMethods, "toNumber", toNumberString);
    defineNative(vm, &vm->stringMethods, "format", formatString);
    defineNative(vm, &vm->stringMethods, "split", splitString);
    defineNative(vm, &vm->stringMethods, "contains", containsString);
    defineNative(vm, &vm->stringMethods, "find", findString);
    defineNative(vm, &vm->stringMethods, "findLast", findLastString);
    defineNative(vm, &vm->stringMethods, "replace", replaceString);
    defineNative(vm, &vm->stringMethods, "lower", lowerString);
    defineNative(vm, &vm->stringMethods, "upper", upperString);
    defineNative(vm, &vm->stringMethods, "startsWith", startsWithString);
    defineNative(vm, &vm->stringMethods, "endsWith", endsWithString);
    defineNative(vm, &vm->stringMethods, "leftStrip", leftStripString);
    defineNative(vm, &vm->stringMethods, "rightStrip", rightStripString);
    defineNative(vm, &vm->stringMethods, "strip", stripString);
    defineNative(vm, &vm->stringMethods, "count", countString);
    defineNative(vm, &vm->stringMethods, "wordCount", wordCountString);
    defineNative(vm, &vm->stringMethods, "toBool", boolNative); // Defined in util
    defineNative(vm, &vm->stringMethods, "title", titleString);
    defineNative(vm, &vm->stringMethods, "repeat", repeatString);
    defineNative(vm, &vm->stringMethods, "isUpper", isUpperString);
    defineNative(vm, &vm->stringMethods, "isLower", isLowerString);
    defineNative(vm, &vm->stringMethods, "collapseSpaces", collapseSpacesString);
    defineNative(vm, &vm->stringMethods, "wrap", wrapString);
    defineNative(vm, &vm->stringMethods, "snakeToCamelCase", snakeToCamelCaseString);
    defineNative(vm, &vm->stringMethods, "camelToSnakeCase", camelToSnakeCaseString);
}
