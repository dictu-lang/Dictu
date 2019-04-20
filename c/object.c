#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ_LIST(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType, true)

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType, false)

static Obj *allocateObject(size_t size, ObjType type, bool isList) {
    Obj *object = (Obj *) reallocate(NULL, 0, size);
    object->type = type;
    object->isDark = false;

    if (!isList) {
        object->next = vm.objects;
        vm.objects = object;
    } else {
        object->next = vm.listObjects;
        vm.listObjects = object;
    }

#ifdef DEBUG_TRACE_GC
    printf("%p allocate %ld for %d\n", (void *)object, size, type);
#endif

    return object;
}

ObjBoundMethod *newBoundMethod(Value receiver, ObjClosure *method) {
    ObjBoundMethod *bound = ALLOCATE_OBJ(ObjBoundMethod,
                                         OBJ_BOUND_METHOD);

    bound->receiver = receiver;
    bound->method = method;
    return bound;
}

ObjClass *newClass(ObjString *name, ObjClass *superclass) {
    ObjClass *klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
    klass->name = name;
    klass->superclass = superclass;
    initTable(&klass->methods);
    return klass;
}

ObjClosure *newClosure(ObjFunction *function) {
    ObjUpvalue **upvalues = ALLOCATE(ObjUpvalue*, function->upvalueCount);
    for (int i = 0; i < function->upvalueCount; i++) {
        upvalues[i] = NULL;
    }

    ObjClosure *closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalueCount = function->upvalueCount;
    return closure;
}

ObjFunction *newFunction(bool isStatic) {
    ObjFunction *function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);

    function->arity = 0;
    function->upvalueCount = 0;
    function->name = NULL;
    function->staticMethod = isStatic;
    initChunk(&function->chunk);
    return function;
}

ObjInstance *newInstance(ObjClass *klass) {
    ObjInstance *instance = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE);
    instance->klass = klass;
    initTable(&instance->fields);
    return instance;
}

ObjNative *newNative(NativeFn function) {
    ObjNative *native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}


ObjNativeVoid *newNativeVoid(NativeFnVoid function) {
    ObjNativeVoid *native = ALLOCATE_OBJ(ObjNativeVoid, OBJ_NATIVE_VOID);
    native->function = function;
    return native;
}

static ObjString *allocateString(char *chars, int length,
                                 uint32_t hash) {
    ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;
    push(OBJ_VAL(string));
    tableSet(&vm.strings, string, NIL_VAL);
    pop();
    return string;
}

ObjList *initList() {
    ObjList *list = ALLOCATE_OBJ(ObjList, OBJ_LIST);
    initValueArray(&list->values);
    return list;
}

ObjDict *initDict() {
    ObjDict *dict = initDictValues(8);
    return dict;
}

ObjFile *initFile() {
    ObjFile *file = ALLOCATE_OBJ(ObjFile, OBJ_FILE);
    return file;
}

static uint32_t hashString(const char *key, int length) {
    uint32_t hash = 2166136261u;

    for (int i = 0; i < length; i++) {
        hash ^= key[i];
        hash *= 16777619;
    }

    return hash;
}

ObjString *takeString(char *chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString *interned = tableFindString(&vm.strings, chars, length,
                                          hash);
    if (interned != NULL) {
        FREE_ARRAY(char, chars, length);
        return interned;
    }

    return allocateString(chars, length, hash);
}

char *removeBackslash(char *string, char c) {
    int write = 0, read = 0;
    while (string[read]) {
        if (string[read] != c) {
            string[write++] = string[read];
        }

        read++;
    }
    string[write] = '\0';

    return string;
}

ObjString *copyString(const char *chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString *interned = tableFindString(&vm.strings, chars, length,
                                          hash);
    if (interned != NULL) return interned;

    //removeBackslash(chars, '\\');

    char *heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return allocateString(heapChars, length, hash);
}

ObjUpvalue *newUpvalue(Value *slot) {
    ObjUpvalue *upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
    upvalue->closed = NIL_VAL;
    upvalue->value = slot;
    upvalue->next = NULL;

    return upvalue;
}

char *objectToString(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_CLASS: {
            ObjClass *klass = AS_CLASS(value);
            char *classString = malloc(sizeof(char) * (klass->name->length + 8));
            snprintf(classString, klass->name->length + 7, "<cls %s>", klass->name->chars);
            return classString;
        }

        case OBJ_BOUND_METHOD: {
            ObjBoundMethod *method = AS_BOUND_METHOD(value);
            char *methodString = malloc(sizeof(char) * (method->method->function->name->length + 17));
            char *methodType = method->method->function->staticMethod ? "<static method %s>" : "<bound method %s>";
            snprintf(methodString, method->method->function->name->length + 17, methodType, method->method->function->name->chars);
            return methodString;
        }

        case OBJ_CLOSURE: {
            ObjClosure *closure = AS_CLOSURE(value);
            char *closureString = malloc(sizeof(char) * (closure->function->name->length + 6));
            snprintf(closureString, closure->function->name->length + 6, "<fn %s>", closure->function->name->chars);
            return closureString;
        }

        case OBJ_FUNCTION: {
            ObjFunction *function = AS_FUNCTION(value);
            char *functionString = malloc(sizeof(char) * (function->name->length + 6));
            snprintf(functionString, function->name->length + 6, "<fn %s>", function->name->chars);
            return functionString;
        }

        case OBJ_INSTANCE: {
            ObjInstance *instance = AS_INSTANCE(value);
            char *instanceString = malloc(sizeof(char) * (instance->klass->name->length + 12));
            snprintf(instanceString, instance->klass->name->length + 12, "<%s instance>", instance->klass->name->chars);
            return instanceString;
        }

        case OBJ_NATIVE_VOID:
        case OBJ_NATIVE: {
            char *nativeString = malloc(sizeof(char) * 12);
            strncpy(nativeString, "<native fn>", 11);
            return nativeString;
        }

        case OBJ_STRING: {
            ObjString *stringObj = AS_STRING(value);
            char *string = malloc(sizeof(char) * stringObj->length + 3);
            snprintf(string, stringObj->length + 3, "'%s'", stringObj->chars);
            return string;
        }

        case OBJ_FILE: {
            ObjFile *file = AS_FILE(value);
            char *fileString = malloc(sizeof(char) * (strlen(file->path) + 8));
            snprintf(fileString, strlen(file->path) + 8, "<file %s>", file->path);
            return fileString;
        }

        case OBJ_LIST: {
            /*
            int size = 50;
            ObjList *list = AS_LIST(value);
            char *listString = malloc(sizeof(char) * size);
            strncpy(listString, "[", 1);

            for (int i = 0; i < list->values.count; ++i) {
                char *element = valueToString(list->values.values[i]);

                if (strlen(element) > (size - strlen(listString))) {
                    printf("realloc\n");
                    size += strlen(element) * 2 + 5;
                    listString = realloc(listString, sizeof(char) * size);
                }

                strncat(listString, element, strlen(element));

                if (i != list->values.count - 1)
                    strncat(listString, ", ", 2);

                free(element);
            }
            strncat(listString, "]", 1);
            return listString;
             */
            int size = 5;
            ObjList *list = AS_LIST(value);
            char *listString = calloc(size, sizeof(char) * size);
            strncpy(listString, "[", 1);

            for (int i = 0; i < list->values.count; ++i) {
                char *element = valueToString(list->values.values[i]);

                int elementSize = strlen(element);
                int listStringSize = strlen(listString);

                if (elementSize > (size - listStringSize - 1)) {
                    if (elementSize > size * 2) {
                        size += elementSize * 2;
                    } else {
                        size *= 2;
                    }

                    char *newB = realloc(listString, sizeof(char) * size);

                    if (newB == NULL) {
                        printf("Error!!\n");
                    }

                    listString = newB;
                }

                strncat(listString, element, size - listStringSize - 1);

                if (i != list->values.count - 1)
                    strncat(listString, ", ", size - listStringSize - 1);

                free(element);
            }

            strncat(listString, "]", size - strlen(listString) - 1);
            return listString;
        }

        case OBJ_DICT: {
            int count = 0;
            ObjDict *dict = AS_DICT(value);
            char *dictString = malloc(sizeof(char) * 10);
            strncpy(dictString, "{", 1);


            for (int i = 0; i < dict->capacity; ++i) {
                dictItem *item = dict->items[i];
                if (!item || item->deleted)
                    continue;

                count++;

                //if (strlen(item->key) > strlen(listString)) {
                //
                //    listString = realloc(listString, sizeof(char) * (strlen(element) * 2 + 4));
                //}


                printf("'%s': ", item->key);
                printValue(item->item);
                if (count != dict->count)
                    printf(", ");
            }
            printf("}");
            break;
        }

        case OBJ_UPVALUE:
            printf("upvalue");
            break;
    }
    return "hi";
}
