#include <stdio.h>
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
    ObjList *list = ALLOCATE_OBJ_LIST(ObjList, OBJ_LIST);
    initValueArray(&list->values);
    return list;
}

ObjDict *initDict() {
    ObjDict *dict = initDictValues(8);
    return dict;
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

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_CLASS:
            printf("<cls %s>", AS_CLASS(value)->name->chars);
            break;

        case OBJ_BOUND_METHOD:
            if (AS_BOUND_METHOD(value)->method->function->staticMethod)
                printf("<static method %s>",
                       AS_BOUND_METHOD(value)->method->function->name->chars);
            else
                printf("<bound method %s>",
                       AS_BOUND_METHOD(value)->method->function->name->chars);

            break;

        case OBJ_CLOSURE:
            printf("<fn %s>", AS_CLOSURE(value)->function->name->chars);
            break;

        case OBJ_FUNCTION:
            printf("<fn %s>", AS_FUNCTION(value)->name->chars);
            break;

        case OBJ_INSTANCE:
            printf("%s instance", AS_INSTANCE(value)->klass->name->chars);
            break;

        case OBJ_NATIVE_VOID:
        case OBJ_NATIVE:
            printf("<native fn>");
            break;

        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;

        case OBJ_LIST: {
            ObjList *list = AS_LIST(value);
            printf("[");
            for (int i = 0; i < list->values.count; ++i) {
                printValue(list->values.values[i]);
                if (i != list->values.count - 1)
                    printf(", ");
            }
            printf("]");
            break;
        }

        case OBJ_DICT: {
            int count = 0;
            ObjDict *dict = AS_DICT(value);
            printf("{");
            for (int i = 0; i < dict->capacity; ++i) {
                dictItem *item = dict->items[i];
                if (!item || item->deleted)
                    continue;

                count++;
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
}
