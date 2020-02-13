#include "lists.h"

static bool pushListItem(int argCount) {
    if (argCount != 2) {
        runtimeError("push() takes 2 arguments (%d given)", argCount);
        return false;
    }

    Value listItem = pop();

    ObjList *list = AS_LIST(pop());
    writeValueArray(&list->values, listItem);
    push(NIL_VAL);

    return true;
}

static bool insertListItem(int argCount) {
    if (argCount != 3) {
        runtimeError("insert() takes 3 arguments (%d given)", argCount);
        return false;
    }

    if (!IS_NUMBER(peek(0))) {
        runtimeError("insert() third argument must be a number");
        return false;
    }


    int index = AS_NUMBER(pop());
    Value insertValue = pop();
    ObjList *list = AS_LIST(pop());

    if (index < 0 || index > list->values.count) {
        runtimeError("Index passed to insert() is out of bounds for the list given");
        return false;
    }

    if (list->values.capacity < list->values.count + 1) {
        int oldCapacity = list->values.capacity;
        list->values.capacity = GROW_CAPACITY(oldCapacity);
        list->values.values = GROW_ARRAY(list->values.values, Value,
                                         oldCapacity, list->values.capacity);
    }

    list->values.count++;

    for (int i = list->values.count - 1; i > index; --i) {
        list->values.values[i] = list->values.values[i - 1];
    }

    list->values.values[index] = insertValue;
    push(NIL_VAL);

    return true;
}

static bool popListItem(int argCount) {
    if (argCount < 1 || argCount > 2) {
        runtimeError("pop() takes either 1 or 2 arguments (%d  given)", argCount);
        return false;
    }

    ObjList *list;
    Value last;

    if (argCount == 1) {
        if (!IS_LIST(peek(0))) {
            runtimeError("pop() only takes a list as an argument");
            return false;
        }

        list = AS_LIST(pop());

        if (list->values.count == 0) {
            runtimeError("pop() called on an empty list");
            return false;
        }

        last = list->values.values[list->values.count - 1];
    } else {
        if (!IS_LIST(peek(1))) {
            runtimeError("pop() only takes a list as an argument");
            return false;
        }

        if (!IS_NUMBER(peek(0))) {
            runtimeError("pop() index argument must be a number");
            return false;
        }

        int index = AS_NUMBER(pop());
        list = AS_LIST(pop());

        if (list->values.count == 0) {
            runtimeError("pop() called on an empty list");
            return false;
        }

        if (index < 0 || index > list->values.count) {
            runtimeError("Index passed to pop() is out of bounds for the list given");
            return false;
        }

        last = list->values.values[index];

        for (int i = index; i < list->values.count - 1; ++i) {
            list->values.values[i] = list->values.values[i + 1];
        }
    }
    list->values.count--;
    push(last);

    return true;
}

static bool containsListItem(int argCount) {
    if (argCount != 2) {
        runtimeError("contains() takes 2 arguments (%d  given)", argCount);
        return false;
    }

    Value search = pop();
    ObjList *list = AS_LIST(pop());

    for (int i = 0; i < list->values.count; ++i) {
        if (valuesEqual(list->values.values[i], search)) {
            push(TRUE_VAL);
            return true;
        }
    }

    push(FALSE_VAL);
    return true;
}

static bool joinListItem(int argCount) {
    if (argCount < 1 || argCount > 2) {
        runtimeError("join() takes either 1 or 2 arguments (%d  given)", argCount);
        return false;
    }

    char *delimiter = ", ";

    if (argCount == 2) {
        if (!IS_STRING(peek(0))) {
            runtimeError("join() only takes a string as an argument");
            return false;
        }

        delimiter = AS_CSTRING(pop());
    }

    ObjList *list = AS_LIST(pop());

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
    push(OBJ_VAL(copyString(fullString, index)));

    if (!IS_STRING(list->values.values[list->values.count - 1])) {
        free(output);
    }
    free(fullString);

    return true;
}

static bool copyListShallow(int argCount) {
    if (argCount != 1) {
        runtimeError("copy() takes 1 argument (%d  given)", argCount);
        return false;
    }

    ObjList *oldList = AS_LIST(peek(0));
    ObjList *newList = copyList(oldList, true);
    pop();
    push(OBJ_VAL(newList));

    return true;
}


static bool copyListDeep(int argCount) {
    if (argCount != 1) {
        runtimeError("deepCopy() takes 1 argument (%d  given)", argCount);
        return false;
    }

    ObjList *oldList = AS_LIST(pop());
    push(OBJ_VAL(copyList(oldList, false)));

    return true;
}

bool listMethods(char *method, int argCount) {
    if (strcmp(method, "push") == 0) {
        return pushListItem(argCount);
    } else if (strcmp(method, "insert") == 0) {
        return insertListItem(argCount);
    } else if (strcmp(method, "pop") == 0) {
        return popListItem(argCount);
    } else if (strcmp(method, "contains") == 0) {
        return containsListItem(argCount);
    } else if (strcmp(method, "join") == 0) {
        return joinListItem(argCount);
    } else if (strcmp(method, "copy") == 0) {
        return copyListShallow(argCount);
    } else if (strcmp(method, "deepCopy") == 0) {
        return copyListDeep(argCount);
    }

    runtimeError("List has no method %s()", method);
    return false;
}
