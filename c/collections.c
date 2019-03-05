#include "collections.h"
#include "vm.h"
#include "memory.h"

static bool pushListItem(int argCount) {
    if (argCount != 2) {
        runtimeError("push() takes two arguments (%d given)", argCount);
        return false;
    }

    ObjList *list = AS_LIST(peek(1));
    writeValueArray(&list->values, pop());
    push(NIL_VAL);

    return true;
}

static bool insertListItem(int argCount) {
    if (argCount != 3) {
        runtimeError("insert() takes three arguments (%d given)", argCount);
        return false;
    }

    if (!IS_NUMBER(peek(0))) {
        runtimeError("insert() third argument must be a number", argCount);
        return false;
    }

    ObjList *list = AS_LIST(peek(2));
    int index = AS_NUMBER(pop());

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

    list->values.values[index] = pop();
    push(NIL_VAL);

    return true;
}

static bool popListItem(int argCount) {
    if (argCount < 1 || argCount > 2) {
        runtimeError("pop() takes either one or two arguments (%d  given)", argCount);
        return false;
    }

    ObjList *list;
    Value last;

    if (argCount == 1) {
        if (!IS_LIST(peek(0))) {
            runtimeError("pop() only takes a list as an argument");
            return false;
        }

        list = AS_LIST(peek(0));

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

        list = AS_LIST(peek(1));

        if (list->values.count == 0) {
            runtimeError("pop() called on an empty list");
            return false;
        }

        int index = AS_NUMBER(pop());

        if (index < 0 || index > list->values.count) {
            runtimeError("Index passed to pop() is out of bounds for the list given");
            return NIL_VAL;
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

bool listMethods(char *method, int argCount) {
    if (strcmp(method, "push") == 0) {
        return pushListItem(argCount);
    } else if (strcmp(method, "insert") == 0) {
        return insertListItem(argCount);
    } else if (strcmp(method, "pop") == 0) {
        return popListItem(argCount);
    }

    return false;
}
