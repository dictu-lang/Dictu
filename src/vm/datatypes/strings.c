#include "strings.h"
#include "../memory.h"
#include "../utf8.h"
#include <stdint.h>
#include <string.h>
#include <wctype.h>
#include <locale.h>



static Value lenString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "len() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);
    if(string->character_len == -1) {
        runtimeError(vm, "String contains invalid UTF-8, consider using byteLen()");
        return EMPTY_VAL;
    }
    return NUMBER_VAL(string->character_len);
}

static Value byteLenString(DictuVM *vm, int argCount, Value *args) {
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
    ObjString *string = AS_STRING(args[0]);

    if(string->character_len == -1) {
     runtimeError(vm, "String contains invalid UTF-8", argCount);
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

        length += utf8size_lazy(replaceStrings[j - 1]);
    }


    int stringLen = string->length + 1;
    char *tmp = string->chars;
    char *tmpFree = tmp;

    int count = 0;
    while ((tmp = utf8str(tmp, "{}"))) {
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
        pos = utf8str(tmp, "{}");

        int tmpLength = pos - tmp;
        int replaceLength = utf8size_lazy(replaceStrings[i]);
        memcpy(newStr + stringLength, tmp, tmpLength);
        memcpy(newStr + stringLength + tmpLength, replaceStrings[i], replaceLength);
        stringLength += tmpLength + replaceLength;
        tmp = pos + 2;
        free(replaceStrings[i]);
    }

    FREE_ARRAY(vm, char*, replaceStrings, argCount);
    memcpy(newStr + stringLength, tmp, utf8size_lazy(tmp));
    newStr[fullLength - 1] = '\0';

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
    if(string->character_len == -1) {
     runtimeError(vm, "String contains invalid UTF-8", argCount);
     return EMPTY_VAL;
     }

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

    char *tmp = string->chars;
    int delimiterLength = utf8size_lazy(delimiter);
    char *token;

    ObjList *list = newList(vm);
    push(vm, OBJ_VAL(list));
    int count = 0;
    char* ptr = string->chars;
    utf8_int32_t cp;
    if (delimiterLength == 0) {
        int tokenIndex = 0;
        for (; tokenIndex < string->character_len && count < maxSplit; tokenIndex++) {
            count++;
          
            char* next_ptr = utf8codepoint(ptr, &cp);

            // *(tmp) = string->chars[tokenIndex];
            // *(tmp + 1) = '\0';
            size_t size = next_ptr - ptr;
            Value str = OBJ_VAL(copyString(vm, ptr, size));
            // Push to stack to avoid GC
            push(vm, str);
            writeValueArray(vm, &list->values, str);
            pop(vm);
            ptr = next_ptr;
        }

        if (tokenIndex != string->character_len && count >= maxSplit) {
            tmp = ptr;
        } else {
            tmp = NULL;
        }
    } else if (maxSplit > 0) {
        do {
            count++;
            token = utf8str(tmp, delimiter);

            Value str = OBJ_VAL(copyString(vm, tmp, token == NULL ? utf8size_lazy(tmp) : ((size_t)(token-tmp))));

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
        Value remainingStr = OBJ_VAL(copyString(vm, tmp, utf8size_lazy(tmp)));

        // Push to stack to avoid GC
        push(vm, remainingStr);
        writeValueArray(vm, &list->values, remainingStr);
        pop(vm);
    }

    pop(vm);

    return OBJ_VAL(list);
}

static Value containsString(DictuVM *vm, int argCount, Value *args) {

    if (argCount != 1) {
        runtimeError(vm, "contains() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }
    {
        ObjString *string = AS_STRING(args[0]);
        if(string->character_len == -1) {
           runtimeError(vm, "String contains invalid UTF-8", argCount);
           return EMPTY_VAL;
       }
    }
    if (!IS_STRING(args[1])) {
        runtimeError(vm, "Argument passed to contains() must be a string");
        return EMPTY_VAL;
    }

    char *string = AS_CSTRING(args[0]);
    char *delimiter = AS_CSTRING(args[1]);

    if (!utf8str(string, delimiter)) {
        return FALSE_VAL;
    }

    return TRUE_VAL;
}

static Value findString(DictuVM *vm, int argCount, Value *args) {
    if (argCount < 1 || argCount > 2) {
        runtimeError(vm, "find() takes either 1 or 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }
    {
        ObjString *string = AS_STRING(args[0]);
        if(string->character_len == -1) {
           runtimeError(vm, "String contains invalid UTF-8", argCount);
           return EMPTY_VAL;
       }
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
        if(i != 0) {
            position +=  utf8len(substr);
        }
        char *result = utf8str(string, substr);
        if (!result) {
            position = -1;
            break;
        }
        position += utf8nlen(string, (result-string));
        string = result + utf8size_lazy(substr);
    }

    return NUMBER_VAL(position);
}

static Value findLastString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "findLast() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }
    {
        ObjString *string = AS_STRING(args[0]);
        if(string->character_len == -1) {
           runtimeError(vm, "String contains invalid UTF-8", argCount);
           return EMPTY_VAL;
       }
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
    {
        ObjString *string = AS_STRING(args[0]);
        if(string->character_len == -1) {
           runtimeError(vm, "String contains invalid UTF-8", argCount);
           return EMPTY_VAL;
       }
    }

    // Pop values off the stack
    Value stringValue = args[0];
    ObjString *to_replace = AS_STRING(args[1]);
    ObjString *replace = AS_STRING(args[2]);
    char *string = AS_CSTRING(stringValue);

    int count = 0;
    int len = to_replace->length;
    int replaceLen = replace->length;

    // Make a copy of the string so we do not modify the original
    char *tmp = string;
    char *tmpFree = tmp;

    // Count the occurrences of the needle so we can determine the size
    // of the string we need to allocate
    while ((tmp = utf8str(tmp, to_replace->chars)) != NULL) {
        count++;
        tmp += len;
    }

    // Reset the pointer
    tmp = tmpFree;

    if (count == 0) {
        return stringValue;
    }

    int length = utf8size_lazy(tmp) - count * (len - replaceLen) + 1;
    char *pos;
    char *newStr = ALLOCATE(vm, char, length);
    int stringLength = 0;

    for (int i = 0; i < count; ++i) {
        pos = utf8str(tmp, to_replace->chars);

        int tmpLength = pos - tmp;
        memcpy(newStr + stringLength, tmp, tmpLength);
        memcpy(newStr + stringLength + tmpLength, replace->chars, replaceLen);
        stringLength += tmpLength + replaceLen;
        tmp = pos + len;
    }

    memcpy(newStr + stringLength, tmp, utf8size_lazy(tmp));
    newStr[length - 1] = '\0';

    return OBJ_VAL(takeString(vm, newStr, length - 1));
}

static Value lowerString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "lower() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);
    if(string->character_len == -1) {
     runtimeError(vm, "String contains invalid UTF-8", argCount);
     return EMPTY_VAL;
    }
    char *temp = ALLOCATE(vm, char, string->length + 1);
    memcpy(temp, string->chars, string->length);
    utf8lwr(temp);
    temp[string->length] = '\0';

    return OBJ_VAL(takeString(vm, temp, string->length));
}

static Value upperString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "upper() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);
    if(string->character_len == -1) {
     runtimeError(vm, "String contains invalid UTF-8", argCount);
     return EMPTY_VAL;
    }

    char *temp = ALLOCATE(vm, char, string->length + 1);
    memcpy(temp, string->chars, string->length);
    temp[string->length] = 0x00;
    utf8upr(temp);

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

    ObjString *string = AS_STRING(args[0]);
    if(string->character_len == -1) {
     runtimeError(vm, "String contains invalid UTF-8", argCount);
     return EMPTY_VAL;
    }

    ObjString *start = AS_STRING(args[1]);
    if(string->length < start->length)
        return BOOL_VAL(false);

    return BOOL_VAL(utf8ncmp(string->chars, start->chars, start->length) == 0);
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
    if(string->character_len == -1) {
     runtimeError(vm, "String contains invalid UTF-8", argCount);
     return EMPTY_VAL;
    }

    ObjString *suffix = AS_STRING(args[1]);

    if (string->length < suffix->length) {
        return FALSE_VAL;
    }

    return BOOL_VAL(utf8cmp(string->chars + (string->length - suffix->length), suffix->chars) == 0);
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
    {
        ObjString *string = AS_STRING(args[0]);
        if(string->character_len == -1) {
           runtimeError(vm, "String contains invalid UTF-8", argCount);
           return EMPTY_VAL;
       }
    }

    char *haystack = AS_CSTRING(args[0]);
    char *needle = AS_CSTRING(args[1]);

    int count = 0;
    while ((haystack = utf8str(haystack, needle))) {
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

    ObjString *string = AS_STRING(args[0]);
    
    if(string->character_len == -1) {
     runtimeError(vm, "String contains invalid UTF-8", argCount);
     return EMPTY_VAL;
    }
    int count = 0;
    int len = string->character_len;
    bool in = false;
    char* ptr = string->chars;

    for (int i = 0; i < len; i++) {
        utf8_int32_t cp;
        ptr = utf8codepoint(ptr, &cp);
        if (iswspace(cp)) {
            in = false;
        } else if(iswalpha(cp)) {
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
    if(string->character_len == -1) {
     runtimeError(vm, "String contains invalid UTF-8", argCount);
     return EMPTY_VAL;
    }

    char *temp = ALLOCATE(vm, char, string->length + 1);
    memcpy(temp, string->chars, string->length);
    char* ptr = temp;
    bool convertNext=true;

    for (int i = 0; i < string->character_len; i++) {
        utf8_int32_t cp;
        char* next_ptr = utf8codepoint(ptr, &cp);
        if(cp==' '){
            convertNext=true;
        }
        else if(convertNext){
            cp = utf8uprcodepoint(cp);
            convertNext=false;
          
        } else {
            cp = utf8lwrcodepoint(cp);
        }
        utf8catcodepoint(ptr, cp, next_ptr -ptr);
        ptr = next_ptr;
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

    utf8cpy(temp, string->chars);
    while (--count > 0) {
        utf8cat(temp, string->chars);
    }

    return OBJ_VAL(takeString(vm, temp, tempLen - 1));
}

static Value isUpperString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "isUpper() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }
    {
        ObjString *string = AS_STRING(args[0]);
        if(string->character_len == -1) {
           runtimeError(vm, "String contains invalid UTF-8", argCount);
           return EMPTY_VAL;
       }
    }


    char *string = AS_CSTRING(args[0]);
    int len = utf8len(string);

    if (len == 0) {
        return BOOL_VAL(false);
    }

    for (int i = 0; i < len; i++) {
        utf8_int32_t cp;
        string = utf8codepoint(string, &cp);
        if (!iswupper(cp) && iswalpha(cp)) {
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
    {
        ObjString *string = AS_STRING(args[0]);
        if(string->character_len == -1) {
           runtimeError(vm, "String contains invalid UTF-8", argCount);
           return EMPTY_VAL;
       }
    }

    char *string = AS_CSTRING(args[0]);
    int len = utf8len(string);

    if (len == 0) {
        return BOOL_VAL(false);
    }

    for (int i = 0; i < len; i++) {
         utf8_int32_t cp;
        string = utf8codepoint(string, &cp);
     
        if (!iswlower(cp) && iswalpha(cp)) {
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
    if(string->character_len == -1) {
       runtimeError(vm, "String contains invalid UTF-8", argCount);
       return EMPTY_VAL;
   }
    char *dest = ALLOCATE(vm, char, string->length + 1);
    char* ptr = string->chars;
    int targetLen = 0;
    int i, j;
    utf8_int32_t last;
    for (i = j = 0; i < string->character_len; ++i) {
        utf8_int32_t cp;
        char* n = utf8codepoint(ptr, &cp);
        
        if (!isspace(cp) || (i > 0 && !isspace(last))) {
            utf8catcodepoint(dest+targetLen, cp, string->length - targetLen);
            targetLen += utf8codepointsize(cp);
        }

        last = cp;
        ptr = n;
    }

    if (targetLen != string->length) {
        dest = SHRINK_ARRAY(vm, dest, char, string->length + 1, targetLen + 1);
    }
    dest[targetLen] = 0x00;

    return OBJ_VAL(takeString(vm, dest, targetLen));
}

static Value wrapString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "wrap() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjString *string = AS_STRING(args[0]);
    if(string->character_len == -1) {
       runtimeError(vm, "String contains invalid UTF-8", argCount);
       return EMPTY_VAL;
   }
    char *temp = ALLOCATE(vm, char, string->length + 1);
    memcpy(temp, string->chars, string->length);
    int len = AS_NUMBER(args[1]);

    int last = 0;
    int count = 0;
    char*ptr = temp;
    for (int cur = 0; cur < string->character_len; cur++, count++) {
        utf8_int32_t cp;
        char* n = utf8codepoint(ptr, &cp);
        // temp[cur] = string->chars[cur];

        if (isspace(cp)) {
            last = ptr - temp;
        } 

        if (count >= len) {
            temp[last] = '\n';
            count = 0;
        }
        ptr = n;
    }

    return OBJ_VAL(takeString(vm, temp, utf8size_lazy(temp)));
}

void declareStringMethods(DictuVM *vm) {
    // Note(Liz3): We need functions from the c stdlib for iswalpha, iswlower, iswupper(the utf8.c 
    // library functions do not work)
    setlocale(LC_ALL, "en_US.UTF-8");
    defineNative(vm, &vm->stringMethods, "len", lenString);
    defineNative(vm, &vm->stringMethods, "byteLen", byteLenString);
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
}
