#include "collections.h"
#include "vm.h"
#include "memory.h"

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

    for (int i = 0; i < list->values.capacity; ++i) {
        if (!list->values.values[i])
            continue;

        if (list->values.values[i] == search) {
            push(TRUE_VAL);
            return true;
        }
    }

    push(FALSE_VAL);
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
    }

    runtimeError("List has no method %s()", method);
    return false;
}

static bool getDictItem(int argCount) {
    if (argCount != 3) {
        runtimeError("get() takes 3 arguments (%d  given)", argCount);
        return false;
    }

    Value defaultValue = pop();

    if (!IS_STRING(peek(0))) {
        runtimeError("Key passed to get() must be a string");
        return false;
    }

    Value key = pop();
    ObjDict *dict = AS_DICT(pop());

    Value ret = searchDict(dict, AS_CSTRING(key));

    if (ret == NIL_VAL) {
        push(defaultValue);
    } else {
        push(ret);
    }

    return true;
}

static bool removeDictItem(int argCount) {
    if (argCount != 2) {
        runtimeError("remove() takes 2 arguments (%d  given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(0))) {
        runtimeError("Key passed to remove() must be a string");
        return false;
    }

    char *key = AS_CSTRING(pop());
    ObjDict *dict = AS_DICT(pop());

    for (int i = 0; i < dict->capacity; ++i) {
        if (!dict->items[i])
            continue;

        if (strcmp(dict->items[i]->key, key) == 0) {
            dict->items[i]->deleted = true;
            dict->count--;
            push(NIL_VAL);

            return true;
        }
    }

    runtimeError("Key '%s' passed to remove() does not exist", key);
    return false;
}

static bool dictItemExists(int argCount) {
    if (argCount != 2) {
        runtimeError("exists() takes 2 arguments (%d  given)", argCount);
        return false;
    }

    if (!IS_STRING(peek(0))) {
        runtimeError("Key passed to exists() must be a string");
        return false;
    }

    char *key = AS_CSTRING(pop());
    ObjDict *dict = AS_DICT(pop());

    for (int i = 0; i < dict->capacity; ++i) {
        if (!dict->items[i])
            continue;

        if (strcmp(dict->items[i]->key, key) == 0) {
            push(TRUE_VAL);
            return true;
        }
    }

    push(FALSE_VAL);
    return true;
}

bool dictMethods(char *method, int argCount) {
    if (strcmp(method, "get") == 0) {
        return getDictItem(argCount);
    } else if (strcmp(method, "remove") == 0) {
        return removeDictItem(argCount);
    } else if (strcmp(method, "exists") == 0) {
        return dictItemExists(argCount);
    }

    runtimeError("Dict has no method %s()", method);
    return false;
}
