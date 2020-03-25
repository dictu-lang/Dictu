#include "lists.h"

static bool pushListItem(VM *vm, int argCount) {
    if (argCount != 2) {
        runtimeError(vm, "push() takes 2 arguments (%d given)", argCount);
        return false;
    }

    Value listItem = peek(vm, 0);
    ObjList *list = AS_LIST(peek(vm, 1));
    writeValueArray(vm, &list->values, listItem);
    pop(vm);
    pop(vm);
    push(vm, NIL_VAL);

    return true;
}

static bool insertListItem(VM *vm, int argCount) {
    if (argCount != 3) {
        runtimeError(vm, "insert() takes 3 arguments (%d given)", argCount);
        return false;
    }

    if (!IS_NUMBER(peek(vm, 0))) {
        runtimeError(vm, "insert() third argument must be a number");
        return false;
    }

    int index = AS_NUMBER(peek(vm, 0));
    Value insertValue = peek(vm, 1);
    ObjList *list = AS_LIST(peek(vm, 2));

    if (index < 0 || index > list->values.count) {
        runtimeError(vm, "Index passed to insert() is out of bounds for the list given");
        return false;
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
    pop(vm);
    pop(vm);
    pop(vm);

    push(vm, NIL_VAL);

    return true;
}

static bool popListItem(VM *vm, int argCount) {
    if (argCount < 1 || argCount > 2) {
        runtimeError(vm, "pop(vm); takes either 1 or 2 arguments (%d  given)", argCount);
        return false;
    }

    ObjList *list;
    Value last;

    if (argCount == 1) {
        if (!IS_LIST(peek(vm, 0))) {
            runtimeError(vm, "pop(vm); only takes a list as an argument");
            return false;
        }

        list = AS_LIST(peek(vm, 0));

        if (list->values.count == 0) {
            runtimeError(vm, "pop(vm); called on an empty list");
            return false;
        }

        last = list->values.values[list->values.count - 1];
    } else {
        if (!IS_LIST(peek(vm, 1))) {
            runtimeError(vm, "pop(vm); only takes a list as an argument");
            return false;
        }

        if (!IS_NUMBER(peek(vm, 0))) {
            runtimeError(vm, "pop(vm); index argument must be a number");
            return false;
        }

        int index = AS_NUMBER(peek(vm, 0));
        list = AS_LIST(peek(vm, 1));

        if (list->values.count == 0) {
            runtimeError(vm, "pop(vm); called on an empty list");
            return false;
        }

        if (index < 0 || index > list->values.count) {
            runtimeError(vm, "Index passed to pop(vm); is out of bounds for the list given");
            return false;
        }

        last = list->values.values[index];

        for (int i = index; i < list->values.count - 1; ++i) {
            list->values.values[i] = list->values.values[i + 1];
        }
    }
    list->values.count--;

    pop(vm);
    if (argCount == 2) {
        pop(vm);
    }

    push(vm, last);

    return true;
}

static bool containsListItem(VM *vm, int argCount) {
    if (argCount != 2) {
        runtimeError(vm, "contains() takes 2 arguments (%d  given)", argCount);
        return false;
    }

    Value search = pop(vm);
    ObjList *list = AS_LIST(pop(vm));

    for (int i = 0; i < list->values.count; ++i) {
        if (valuesEqual(list->values.values[i], search)) {
            push(vm, TRUE_VAL);
            return true;
        }
    }

    push(vm, FALSE_VAL);
    return true;
}

static bool joinListItem(VM *vm, int argCount) {
    if (argCount < 1 || argCount > 2) {
        runtimeError(vm, "join() takes either 1 or 2 arguments (%d  given)", argCount);
        return false;
    }

    char *delimiter = ", ";
    ObjList *list;

    if (argCount == 2) {
        if (!IS_STRING(peek(vm, 0))) {
            runtimeError(vm, "join() only takes a string as an argument");
            return false;
        }

        delimiter = AS_CSTRING(peek(vm, 0));
        list = AS_LIST(peek(vm, 1));
    } else {
        list = AS_LIST(peek(vm, 0));
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

    pop(vm);
    if (argCount == 2) {
        pop(vm);
    }

    push(vm, OBJ_VAL(copyString(vm, fullString, index)));

    free(fullString);

    return true;
}

static bool copyListShallow(VM *vm, int argCount) {
    if (argCount != 1) {
        runtimeError(vm, "copy() takes 1 argument (%d  given)", argCount);
        return false;
    }

    ObjList *oldList = AS_LIST(peek(vm, 0));
    ObjList *newList = copyList(vm, oldList, true);
    pop(vm);
    push(vm, OBJ_VAL(newList));

    return true;
}


static bool copyListDeep(VM *vm, int argCount) {
    if (argCount != 1) {
        runtimeError(vm, "deepCopy() takes 1 argument (%d  given)", argCount);
        return false;
    }

    ObjList *oldList = AS_LIST(peek(vm, 0));
    ObjList *newList = copyList(vm, oldList, false);
    pop(vm);
    push(vm, OBJ_VAL(newList));


    return true;
}

bool listMethods(VM *vm, char *method, int argCount) {
    if (strcmp(method, "push") == 0) {
        return pushListItem(vm, argCount);
    } else if (strcmp(method, "insert") == 0) {
        return insertListItem(vm, argCount);
    } else if (strcmp(method, "pop") == 0) {
        return popListItem(vm, argCount);
    } else if (strcmp(method, "contains") == 0) {
        return containsListItem(vm, argCount);
    } else if (strcmp(method, "join") == 0) {
        return joinListItem(vm, argCount);
    } else if (strcmp(method, "copy") == 0) {
        return copyListShallow(vm, argCount);
    } else if (strcmp(method, "deepCopy") == 0) {
        return copyListDeep(vm, argCount);
    }

    runtimeError(vm, "List has no method %s()", method);
    return false;
}
