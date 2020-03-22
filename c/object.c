#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(vm, type, objectType) \
    (type*)allocateObject(vm, sizeof(type), objectType)

static Obj *allocateObject(VM *vm, size_t size, ObjType type) {
    Obj *object;
    object = (Obj *) reallocate(vm, NULL, 0, size);

    object->type = type;
    object->isDark = false;
    object->next = vm->objects;
    vm->objects = object;

#ifdef DEBUG_TRACE_GC
    printf("%p allocate %zd for %d\n", (void *)object, size, type);
#endif

    return object;
}

ObjBoundMethod *newBoundMethod(VM *vm, Value receiver, ObjClosure *method) {
    ObjBoundMethod *bound = ALLOCATE_OBJ(vm, ObjBoundMethod,
                                         OBJ_BOUND_METHOD);

    bound->receiver = receiver;
    bound->method = method;
    return bound;
}

ObjClass *newClass(VM *vm, ObjString *name, ObjClass *superclass) {
    ObjClass *klass = ALLOCATE_OBJ(vm, ObjClass, OBJ_CLASS);
    klass->name = name;
    klass->superclass = superclass;
    initTable(&klass->methods);
    return klass;
}

ObjClassNative *newClassNative(VM *vm, ObjString *name) {
    ObjClassNative *klass = ALLOCATE_OBJ(vm, ObjClassNative, OBJ_NATIVE_CLASS);
    klass->name = name;
    initTable(&klass->methods);
    return klass;
}

ObjTrait *newTrait(VM *vm, ObjString *name) {
    ObjTrait *trait = ALLOCATE_OBJ(vm, ObjTrait, OBJ_TRAIT);
    trait->name = name;
    initTable(&trait->methods);
    return trait;
}

ObjClosure *newClosure(VM *vm, ObjFunction *function) {
    ObjUpvalue **upvalues = ALLOCATE(vm, ObjUpvalue*, function->upvalueCount);
    for (int i = 0; i < function->upvalueCount; i++) {
        upvalues[i] = NULL;
    }

    ObjClosure *closure = ALLOCATE_OBJ(vm, ObjClosure, OBJ_CLOSURE);
    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalueCount = function->upvalueCount;
    return closure;
}

ObjFunction *newFunction(VM *vm, bool isStatic) {
    ObjFunction *function = ALLOCATE_OBJ(vm, ObjFunction, OBJ_FUNCTION);

    function->arity = 0;
    function->arityOptional = 0;
    function->upvalueCount = 0;
    function->name = NULL;
    function->staticMethod = isStatic;
    initChunk(vm, &function->chunk);
    return function;
}

ObjInstance *newInstance(VM *vm, ObjClass *klass) {
    ObjInstance *instance = ALLOCATE_OBJ(vm, ObjInstance, OBJ_INSTANCE);
    instance->klass = klass;
    initTable(&instance->fields);
    return instance;
}

ObjNative *newNative(VM *vm, NativeFn function) {
    ObjNative *native = ALLOCATE_OBJ(vm, ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}

static ObjString *allocateString(VM *vm, char *chars, int length,
                                 uint32_t hash) {
    ObjString *string = ALLOCATE_OBJ(vm, ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;
    push(vm, OBJ_VAL(string));
    tableSet(vm, &vm->strings, string, NIL_VAL);
    pop(vm);
    return string;
}

ObjList *initList(VM *vm) {
    ObjList *list = ALLOCATE_OBJ(vm, ObjList, OBJ_LIST);
    initValueArray(&list->values);
    return list;
}

ObjDict *initDict(VM *vm) {
    ObjDict *dict = ALLOCATE_OBJ(vm, ObjDict, OBJ_DICT);
    initDictValues(dict, 8);
    return dict;
}

ObjSet *initSet(VM *vm) {
    ObjSet *set = ALLOCATE_OBJ(vm, ObjSet, OBJ_SET);
    initSetValues(set, 8);
    return set;
}

ObjFile *initFile(VM *vm) {
    ObjFile *file = ALLOCATE_OBJ(vm, ObjFile, OBJ_FILE);
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

ObjString *takeString(VM *vm, char *chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString *interned = tableFindString(&vm->strings, chars, length,
                                          hash);
    if (interned != NULL) {
        FREE_ARRAY(vm, char, chars, length);
        return interned;
    }

    return allocateString(vm, chars, length, hash);
}

ObjString *copyString(VM *vm, const char *chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString *interned = tableFindString(&vm->strings, chars, length,
                                          hash);
    if (interned != NULL) return interned;

    char *heapChars = ALLOCATE(vm, char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return allocateString(vm, heapChars, length, hash);
}

ObjUpvalue *newUpvalue(VM *vm, Value *slot) {
    ObjUpvalue *upvalue = ALLOCATE_OBJ(vm, ObjUpvalue, OBJ_UPVALUE);
    upvalue->closed = NIL_VAL;
    upvalue->value = slot;
    upvalue->next = NULL;

    return upvalue;
}

char *objectToString(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_NATIVE_CLASS:
        case OBJ_CLASS: {
            ObjClass *klass = AS_CLASS(value);
            char *classString = malloc(sizeof(char) * (klass->name->length + 8));
            snprintf(classString, klass->name->length + 7, "<cls %s>", klass->name->chars);
            return classString;
        }

        case OBJ_TRAIT: {
            ObjTrait *trait = AS_TRAIT(value);
            char *traitString = malloc(sizeof(char) * (trait->name->length + 10));
            snprintf(traitString, trait->name->length + 9, "<trait %s>", trait->name->chars);
            return traitString;
        }

        case OBJ_BOUND_METHOD: {
            ObjBoundMethod *method = AS_BOUND_METHOD(value);
            char *methodString;

            if (method->method->function->name != NULL) {
                methodString = malloc(sizeof(char) * (method->method->function->name->length + 17));
                char *methodType = method->method->function->staticMethod ? "<static method %s>" : "<bound method %s>";
                snprintf(methodString, method->method->function->name->length + 17, methodType, method->method->function->name->chars);
            } else {
                methodString = malloc(sizeof(char) * 16);
                char *methodType = method->method->function->staticMethod ? "<static method>" : "<bound method>";
                snprintf(methodString, 16, "%s", methodType);
            }

            return methodString;
        }

        case OBJ_CLOSURE: {
            ObjClosure *closure = AS_CLOSURE(value);
            char *closureString;

            if (closure->function->name != NULL) {
                closureString = malloc(sizeof(char) * (closure->function->name->length + 6));
                snprintf(closureString, closure->function->name->length + 6, "<fn %s>", closure->function->name->chars);
            } else {
                closureString = malloc(sizeof(char) * 9);
                snprintf(closureString, 9, "%s", "<script>");
            }

            return closureString;
        }

        case OBJ_FUNCTION: {
            ObjFunction *function = AS_FUNCTION(value);
            char *functionString;

            if (function->name != NULL) {
                functionString = malloc(sizeof(char) * (function->name->length + 6));
                snprintf(functionString, function->name->length + 6, "<fn %s>", function->name->chars);
            } else {
                functionString = malloc(sizeof(char) * 5);
                snprintf(functionString, 5, "%s", "<fn>");
            }

            return functionString;
        }

        case OBJ_INSTANCE: {
            ObjInstance *instance = AS_INSTANCE(value);
            char *instanceString = malloc(sizeof(char) * (instance->klass->name->length + 12));
            snprintf(instanceString, instance->klass->name->length + 12, "<%s instance>", instance->klass->name->chars);
            return instanceString;
        }

        case OBJ_NATIVE: {
            char *nativeString = malloc(sizeof(char) * 12);
            snprintf(nativeString, 12, "%s", "<native fn>");
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
            int size = 50;
            ObjList *list = AS_LIST(value);
            char *listString = calloc(size, sizeof(char));
            int listStringLength = snprintf(listString, 2, "%s", "[");

            for (int i = 0; i < list->values.count; ++i) {
                char *element = valueToString(list->values.values[i]);

                int elementSize = strlen(element);

                if (elementSize > (size - listStringLength - 3)) {
                    if (elementSize > size * 2) {
                        size += elementSize * 2 + 3;
                    } else {
                        size = size * 2 + 3;
                    }

                    char *newB = realloc(listString, sizeof(char) * size);

                    if (newB == NULL) {
                        printf("Unable to allocate memory\n");
                        exit(71);
                    }

                    listString = newB;
                }

                listStringLength += snprintf(listString + listStringLength, size - listStringLength, "%s", element);

                free(element);

                if (i != list->values.count - 1) {
                    listStringLength += snprintf(listString + listStringLength, size - listStringLength, ", ");
                }
            }

            snprintf(listString + listStringLength, size - listStringLength, "]");
            return listString;
        }

        case OBJ_DICT: {
            int count = 0;
            int size = 50;
            ObjDict *dict = AS_DICT(value);
            char *dictString = malloc(sizeof(char) * size);
            int dictStringLength = snprintf(dictString, size, "%s", "{");

            for (int i = 0; i < dict->capacity; ++i) {
                dictItem *item = dict->items[i];
                if (!item || item->deleted)
                    continue;

                count++;


                int keySize = strlen(item->key) + 5;

                if (keySize > (size - dictStringLength - 1)) {
                    if (keySize > size * 2) {
                        size += keySize * 2;
                    } else {
                        size *= 2;
                    }

                    char *newB = realloc(dictString, sizeof(char) * size);

                    if (newB == NULL) {
                        printf("Unable to allocate memory\n");
                        exit(71);
                    }

                    dictString = newB;
                }

                dictStringLength += snprintf(dictString + dictStringLength, size - dictStringLength, "\"%s\": ", item->key);

                char *element = valueToString(item->item);
                int elementSize = strlen(element);

                if (elementSize > (size - dictStringLength - 3)) {
                    if (elementSize > size * 2) {
                        size += elementSize * 2 + 3;
                    } else {
                        size = size * 2 + 3;
                    }

                    char *newB = realloc(dictString, sizeof(char) * size);

                    if (newB == NULL) {
                        printf("Unable to allocate memory\n");
                        exit(71);
                    }

                    dictString = newB;
                }

                dictStringLength += snprintf(dictString + dictStringLength, size - dictStringLength, "%s", element);

                free(element);

                if (count != dict->count) {
                    dictStringLength += snprintf(dictString + dictStringLength, size - dictStringLength, ", ");
                }
            }

            snprintf(dictString + dictStringLength, size - dictStringLength, "}");
            return dictString;
        }

        case OBJ_SET: {
            int count = 0;
            int size = 50;
            ObjSet *set = AS_SET(value);
            char *setString = malloc(sizeof(char) * size);
            int setStringLength = snprintf(setString, size, "%s", "{");

            for (int i = 0; i < set->capacity; ++i) {
                setItem *item = set->items[i];
                if (!item || item->deleted)
                    continue;

                count++;

                char *element = item->item->chars;
                int elementSize = item->item->length;

                if (elementSize > (size - setStringLength - 5)) {
                    if (elementSize > size * 2) {
                        size += elementSize * 2 + 5;
                    } else {
                        size = size * 2 + 5;
                    }

                    char *newB = realloc(setString, sizeof(char) * size);

                    if (newB == NULL) {
                        printf("Unable to allocate memory\n");
                        exit(71);
                    }

                    setString = newB;
                }

                setStringLength += snprintf(setString + setStringLength, size - setStringLength, "'%s'", element);

                if (count != set->count) {
                    setStringLength += snprintf(setString + setStringLength, size - setStringLength, ", ");
                }
            }

            snprintf(setString + setStringLength, size - setStringLength, "}");
            return setString;
        }

        case OBJ_UPVALUE: {
            char *nativeString = malloc(sizeof(char) * 8);
            snprintf(nativeString, 8, "%s", "upvalue");
            return nativeString;
        }
    }

    char *unknown = malloc(sizeof(char) * 9);
    snprintf(unknown, 8, "%s", "unknown");
    return unknown;
}
