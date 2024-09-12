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

    char *valueString = listToString(args[0]);

    ObjString *string = copyString(vm, valueString, strlen(valueString));
    free(valueString);

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
        runtimeError(vm, "extend() argument must be a list");
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
        runtimeError(vm, "insert() second argument must be a number");
        return EMPTY_VAL;
    }

    ObjList *list = AS_LIST(args[0]);
    Value insertValue = args[1];
    int index = AS_NUMBER(args[2]);

    if (index < 0) {
        index = list->values.count + index+1;
    }
    if (index < 0 || index > list->values.count) {
        runtimeError(vm, "Index passed to insert() is out of bounds for the list given");
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
        runtimeError(vm, "pop() takes either 0 or 1 arguments (%d given)", argCount);
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
            runtimeError(vm, "pop() index argument must be a number");
            return EMPTY_VAL;
        }

        int index = AS_NUMBER(args[1]);
        if (index < 0) {
            index = list->values.count + index;
        }
        if (index < 0 || index > list->values.count) {
            runtimeError(vm, "Index passed to pop() is out of bounds for the list given");
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
        runtimeError(vm, "join() takes 1 optional argument (%d given)", argCount);
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
        if (IS_STRING(list->values.values[j])) {
            output = AS_CSTRING(list->values.values[j]);
        } else {
            output = valueToString(list->values.values[j]);
        }
        int elementLength = strlen(output);

        fullString = GROW_ARRAY(vm, fullString, char, length, length + elementLength + delimiterLength);

        memcpy(fullString + length, output, elementLength);
        if (!IS_STRING(list->values.values[j])) {
            free(output);
        }
        length += elementLength;
        memcpy(fullString + length, delimiter, delimiterLength);
        length += delimiterLength;
    }

    // Outside the loop as we do not want the append the delimiter on the last element
    if (IS_STRING(list->values.values[list->values.count - 1])) {
        output = AS_CSTRING(list->values.values[list->values.count - 1]);
    } else {
        output = valueToString(list->values.values[list->values.count - 1]);
    }

    int elementLength = strlen(output);
    fullString = GROW_ARRAY(vm, fullString, char, length, length + elementLength + 1);
    memcpy(fullString + length, output, elementLength);
    length += elementLength;

    fullString[length] = '\0';

    if (!IS_STRING(list->values.values[list->values.count - 1])) {
        free(output);
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


int compareString(ObjString *operandOne, ObjString *operandTwo) {
    return strcmp(operandOne->chars, operandTwo->chars);
}

static int partitionStringList(ObjList *arr, int start, int end) {
    int pivot_index = (int) floor(start + end) / 2;

    ObjString *pivot = AS_STRING(arr->values.values[pivot_index]);

    int i = start - 1;
    int j = end + 1;

    for (;;) {
        do {
            i = i + 1;
        } while (compareString(AS_STRING(arr->values.values[i]), pivot) < 0);

        do {
            j = j - 1;
        } while (compareString(AS_STRING(arr->values.values[j]), pivot) > 0);

        if (i >= j) {
            return j;
        }

        // Swap arr[i] with arr[j]
        Value temp = arr->values.values[i];

        arr->values.values[i] = arr->values.values[j];
        arr->values.values[j] = temp;
    }
}


static int partitionNumberList(ObjList *arr, int start, int end) {
    int pivot_index = (int) floor(start + end) / 2;

    double pivot = AS_NUMBER(arr->values.values[pivot_index]);

    int i = start - 1;
    int j = end + 1;

    for (;;) {
        do {
            i = i + 1;
        } while (AS_NUMBER(arr->values.values[i]) < pivot);

        do {
            j = j - 1;
        } while (AS_NUMBER(arr->values.values[j]) > pivot);

        if (i >= j) {
            return j;
        }

        // Swap arr[i] with arr[j]
        Value temp = arr->values.values[i];

        arr->values.values[i] = arr->values.values[j];
        arr->values.values[j] = temp;
    }
}

// Implementation of Quick Sort using the Hoare
// Partition scheme.
// Best Case O(n log n)
// Worst Case O(n^2) (If the list is already sorted.) 
static void quickSort(ObjList *arr, int start, int end, int (*partition)(ObjList *, int, int)) {
    while (start < end) {
        int part = partition(arr, start, end);

        // Recurse for the smaller halve.
        if (part - start < end - part) {
            quickSort(arr, start, part, partition);

            start = start + 1;
        } else {
            quickSort(arr, part + 1, end, partition);

            end = end - 1;
        }
    }
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

static Value sortList(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "sort() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjList *list = AS_LIST(args[0]);

    if (isNumberList(list)) {
        quickSort(list, 0, list->values.count - 1, &partitionNumberList);
        return NIL_VAL;
    }

    if (isStringList(list)) {
        quickSort(list, 0, list->values.count - 1, &partitionStringList);
        return NIL_VAL;
    }

    runtimeError(vm,
                 "sort() takes lists with either numbers or string (other types and heterotypic lists are not supported)");
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
