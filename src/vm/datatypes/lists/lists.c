#include <math.h>

#include "lists.h"

/*
 * Note: We should try to implement everything we can in C
 *       rather than in the host language as C will always
 *       be faster than Dictu, and there will be extra work
 *       at startup running the Dictu code, however compromises
 *       must be made due to the fact the VM is not re-enterable
 */

#include "list-source.h"

static Value toStringList(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "toString() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    int valueStringLen = 0;
    char *valueString = listToString(vm, args[0], &valueStringLen);

    ObjString *string = copyString(vm, valueString, valueStringLen);
    FREE_ARRAY(vm, char, valueString, valueStringLen + 1);

    return OBJ_VAL(string);
}

static Value lenList(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "len() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjList *list = AS_LIST(args[0]);
    return NUMBER_VAL(list->values.count);
}

static Value extendList(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "extend() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_LIST(args[1])) {
        int valLength = 0;
        char *val = valueTypeToString(vm, args[1], &valLength);
        runtimeError(vm, "extend() argument must be a list, got '%s'.", val);
        FREE_ARRAY(vm, char, val, valLength + 1);
        return EMPTY_VAL;
    }

    ObjList *list = AS_LIST(args[0]);
    ObjList *listArgument = AS_LIST(args[1]);

    for (int i = 0; i < listArgument->values.count; i++) {
        writeValueArray(vm, &list->values, listArgument->values.values[i]);
    }

    return NIL_VAL;
}

static Value pushListItem(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "push() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjList *list = AS_LIST(args[0]);
    writeValueArray(vm, &list->values, args[1]);

    return NIL_VAL;
}

static Value insertListItem(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "insert() takes 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[2])) {
        int valLength = 0;
        char *val = valueTypeToString(vm, args[2], &valLength);
        runtimeError(vm, "insert() second argument must be a number, got '%s'.", val);
        FREE_ARRAY(vm, char, val, valLength + 1);
        return EMPTY_VAL;
    }

    ObjList *list = AS_LIST(args[0]);
    Value insertValue = args[1];
    int index = AS_NUMBER(args[2]);

    if (index < 0) {
        index = list->values.count + index+1;
    }
    if (index < 0 || index > list->values.count) {
        runtimeError(vm, "insert() index %d out of bounds for list of length %d.", index, list->values.count);
        return EMPTY_VAL;
    }

    if (list->values.capacity < list->values.count + 1) {
        int oldCapacity = list->values.capacity;
        list->values.capacity = GROW_CAPACITY(oldCapacity);
        list->values.values = GROW_ARRAY(vm, list->values.values, Value,
                                         oldCapacity, list->values.capacity);
    }

    list->values.count++;

    for (int i = list->values.count - 1; i > index; --i) {
        list->values.values[i] = list->values.values[i - 1];
    }

    list->values.values[index] = insertValue;

    return NIL_VAL;
}

static Value popListItem(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0 && argCount != 1) {
        runtimeError(vm, "pop() takes 0 or 1 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjList *list = AS_LIST(args[0]);

    if (list->values.count == 0) {
        runtimeError(vm, "pop() called on an empty list");
        return EMPTY_VAL;
    }

    Value element;

    if (argCount == 1) {
        if (!IS_NUMBER(args[1])) {
            int valLength = 0;
            char *val = valueTypeToString(vm, args[1], &valLength);
            runtimeError(vm, "pop() index argument must be a number, got '%s'.", val);
            FREE_ARRAY(vm, char, val, valLength + 1);
            return EMPTY_VAL;
        }

        int index = AS_NUMBER(args[1]);
        if (index < 0) {
            index = list->values.count + index;
        }
        if (index < 0 || index > list->values.count) {
            runtimeError(vm, "pop() index %d out of bounds for list of length %d.", index, list->values.count);
            return EMPTY_VAL;
        }

        element = list->values.values[index];

        for (int i = index; i < list->values.count - 1; ++i) {
            list->values.values[i] = list->values.values[i + 1];
        }
    } else {
        element = list->values.values[list->values.count - 1];
    }

    list->values.count--;

    return element;
}

static Value removeListItem(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "remove() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjList *list = AS_LIST(args[0]);
    Value remove = args[1];
    bool found = false;

    if (list->values.count == 0) {
        runtimeError(vm, "Value passed to remove() does not exist within an empty list");
        return EMPTY_VAL;
    }

    if (list->values.count > 1) {
        for (int i = 0; i < list->values.count - 1; i++) {
            if (!found && valuesEqual(remove, list->values.values[i])) {
                found = true;
            }

            // If we have found the value, shuffle the array
            if (found) {
                list->values.values[i] = list->values.values[i + 1];
            }
        }

        // Check if it's the last element
        if (!found && valuesEqual(remove, list->values.values[list->values.count - 1])) {
            found = true;
        }
    } else {
        if (valuesEqual(remove, list->values.values[0])) {
            found = true;
        }
    }

    if (found) {
        list->values.count--;
        return NIL_VAL;
    }

    runtimeError(vm, "Value passed to remove() does not exist within the list");
    return EMPTY_VAL;
}

static Value containsListItem(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "contains() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjList *list = AS_LIST(args[0]);
    Value search = args[1];

    for (int i = 0; i < list->values.count; ++i) {
        if (valuesEqual(list->values.values[i], search)) {
            return TRUE_VAL;
        }
    }

    return FALSE_VAL;
}

static Value joinListItem(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0 && argCount != 1) {
        runtimeError(vm, "join() takes 0 or 1 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjList *list = AS_LIST(args[0]);

    if (list->values.count == 0) {
        return OBJ_VAL(copyString(vm, "", 0));
    }

    char *delimiter = ", ";

    if (argCount == 1) {
        if (!IS_STRING(args[1])) {
            runtimeError(vm, "join() only takes a string as an argument");
            return EMPTY_VAL;
        }

        delimiter = AS_CSTRING(args[1]);
    }

    char *output;
    char *fullString = NULL;
    int length = 0;
    int delimiterLength = strlen(delimiter);

    for (int j = 0; j < list->values.count - 1; ++j) {
        int elementLength;
        if (IS_STRING(list->values.values[j])) {
            output = AS_CSTRING(list->values.values[j]);
            elementLength = AS_STRING(list->values.values[j])->length;
        } else {
            output = valueToString(vm, list->values.values[j], &elementLength);
        }

        fullString = GROW_ARRAY(vm, fullString, char, length, length + elementLength + delimiterLength);

        memcpy(fullString + length, output, elementLength);
        if (!IS_STRING(list->values.values[j])) {
            FREE_ARRAY(vm, char, output, elementLength + 1);
        }
        length += elementLength;
        memcpy(fullString + length, delimiter, delimiterLength);
        length += delimiterLength;
    }

    // Outside the loop as we do not want the append the delimiter on the last element
    int elementLength;
    if (IS_STRING(list->values.values[list->values.count - 1])) {
        output = AS_CSTRING(list->values.values[list->values.count - 1]);
        elementLength = AS_STRING(list->values.values[list->values.count - 1])->length;
    } else {
        output = valueToString(vm, list->values.values[list->values.count - 1], &elementLength);
    }

    fullString = GROW_ARRAY(vm, fullString, char, length, length + elementLength + 1);
    memcpy(fullString + length, output, elementLength);
    length += elementLength;

    fullString[length] = '\0';

    if (!IS_STRING(list->values.values[list->values.count - 1])) {
        FREE_ARRAY(vm, char, output, elementLength + 1);
    }

    return OBJ_VAL(takeString(vm, fullString, length));
}

static Value copyListShallow(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "copy() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjList *oldList = AS_LIST(args[0]);
    ObjList *list = copyList(vm, oldList, true);
    return OBJ_VAL(list);
}

static Value copyListDeep(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "deepCopy() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjList *oldList = AS_LIST(args[0]);
    ObjList *list = copyList(vm, oldList, false);

    return OBJ_VAL(list);
}

bool isNumberList(ObjList *list) {
    for (int i = 0; i < list->values.count; i++) {
        if (!IS_NUMBER(list->values.values[i])) {
            return false;
        }
    }
    return true;
}

bool isStringList(ObjList *list) {
    for (int i = 0; i < list->values.count; i++) {
        if (!IS_STRING(list->values.values[i])) {
            return false;
        }
    }
    return true;
}

int compareNumbers(const void* a, const void* b) {
    Value aVal = *(Value *)a;
    Value bVal = *(Value *)b;

    return AS_NUMBER(aVal) - AS_NUMBER(bVal);
}

int compareString(const void* a, const void* b) {
    Value aVal = *(Value *)a;
    Value bVal = *(Value *)b;

    return utf8cmp(AS_CSTRING(aVal), AS_CSTRING(bVal));
}

static Value sortList(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "sort() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjList *list = AS_LIST(args[0]);

    if (isNumberList(list)) {
        qsort(list->values.values, list->values.count, sizeof(*list->values.values), compareNumbers);
        return NIL_VAL;
    }

    if (isStringList(list)) {
        qsort(list->values.values, list->values.count, sizeof(*list->values.values), compareString);
        return NIL_VAL;
    }

    runtimeError(vm,
                 "sort() takes lists with either numbers or string (other types and heterogeneous lists are not supported)");
    return EMPTY_VAL;
}

static Value reverseList(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "reverse() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjList *list = AS_LIST(args[0]);
    int listLength = list->values.count;

    for (int i = 0; i < listLength / 2; i++) {
        Value temp = list->values.values[i];
        list->values.values[i] = list->values.values[listLength - i - 1];
        list->values.values[listLength - i - 1] = temp;
    }

    return NIL_VAL;
}

void declareListMethods(DictuVM *vm) {
    defineNative(vm, &vm->listMethods, "toString", toStringList);
    defineNative(vm, &vm->listMethods, "len", lenList);
    defineNative(vm, &vm->listMethods, "extend", extendList);
    defineNative(vm, &vm->listMethods, "push", pushListItem);
    defineNative(vm, &vm->listMethods, "insert", insertListItem);
    defineNative(vm, &vm->listMethods, "pop", popListItem);
    defineNative(vm, &vm->listMethods, "remove", removeListItem);
    defineNative(vm, &vm->listMethods, "contains", containsListItem);
    defineNative(vm, &vm->listMethods, "join", joinListItem);
    defineNative(vm, &vm->listMethods, "copy", copyListShallow);
    defineNative(vm, &vm->listMethods, "deepCopy", copyListDeep);
    defineNative(vm, &vm->listMethods, "toBool", boolNative); // Defined in util
    defineNative(vm, &vm->listMethods, "sort", sortList);
    defineNative(vm, &vm->listMethods, "reverse", reverseList);

    dictuInterpret(vm, "List", DICTU_LIST_SOURCE);

    Value List;
    tableGet(&vm->modules, copyString(vm, "List", 4), &List);

    ObjModule *ListModule = AS_MODULE(List);
    push(vm, List);
    tableAddAll(vm, &ListModule->values, &vm->listMethods);
    pop(vm);
}
