#include "lists.h"


static Value pushListItem(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "push() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjList *list = AS_LIST(args[0]);
    writeValueArray(vm, &list->values, args[1]);

    return NIL_VAL;
}

static Value insertListItem(VM *vm, int argCount, Value *args) {
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

static Value popListItem(VM *vm, int argCount, Value *args) {
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

static Value containsListItem(VM *vm, int argCount, Value *args) {
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

static Value joinListItem(VM *vm, int argCount, Value *args) {
    if (argCount != 0 && argCount != 1) {
        runtimeError(vm, "join() takes 1 optional argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjList *list = AS_LIST(args[0]);
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
    int index = 0;
    int delimiterLength = strlen(delimiter);

    for (int j = 0; j < list->values.count - 1; ++j) {
        if (IS_STRING(list->values.values[j])) {
            output = AS_CSTRING(list->values.values[j]);
        } else {
            output = valueToString(list->values.values[j]);
        }
        int elementLength = strlen(output);
        fullString = realloc(fullString, index + elementLength + delimiterLength + 1);

        memcpy(fullString + index, output, elementLength);
        if (!IS_STRING(list->values.values[j])) {
            free(output);
        }
        index += elementLength;
        memcpy(fullString + index, delimiter, delimiterLength);
        index += delimiterLength;
    }

    // Outside the loop as we do not want the append the delimiter on the last element
    if (IS_STRING(list->values.values[list->values.count - 1])) {
        output = AS_CSTRING(list->values.values[list->values.count - 1]);
    } else {
        output = valueToString(list->values.values[list->values.count - 1]);
    }

    int elementLength = strlen(output);
    fullString = realloc(fullString, index + elementLength + 1);
    memcpy(fullString + index, output, elementLength);
    index += elementLength;

    fullString[index] = '\0';

    if (!IS_STRING(list->values.values[list->values.count - 1])) {
        free(output);
    }

    Value ret = OBJ_VAL(copyString(vm, fullString, index));
    free(fullString);
    return ret;
}

static Value copyListShallow(VM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "copy() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjList *oldList = AS_LIST(args[0]);
    ObjList *newList = copyList(vm, oldList, true);
    return OBJ_VAL(newList);
}

static Value copyListDeep(VM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "deepCopy() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjList *oldList = AS_LIST(args[0]);
    ObjList *newList = copyList(vm, oldList, false);

    return OBJ_VAL(newList);
}

static Value lenList(VM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "len() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjList *list = AS_LIST(args[0]);
    return NUMBER_VAL(list->values.count);
}

void declareListMethods(VM *vm) {
    defineNative(vm, &vm->listMethods, "len", lenList);
    defineNative(vm, &vm->listMethods, "push", pushListItem);
    defineNative(vm, &vm->listMethods, "insert", insertListItem);
    defineNative(vm, &vm->listMethods, "pop", popListItem);
    defineNative(vm, &vm->listMethods, "contains", containsListItem);
    defineNative(vm, &vm->listMethods, "join", joinListItem);
    defineNative(vm, &vm->listMethods, "copy", copyListShallow);
    defineNative(vm, &vm->listMethods, "deepCopy", copyListDeep);
}
