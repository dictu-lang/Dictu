#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"
#include "utf8.h"

#define ALLOCATE_OBJ(vm, type, objectType) \
    (type*)allocateObject(vm, sizeof(type), objectType)

static Obj *allocateObject(DictuVM *vm, size_t size, ObjType type) {
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

ObjModule *newModule(DictuVM *vm, ObjString *name) {
    Value moduleVal;
    if (tableGet(&vm->modules, name, &moduleVal)) {
        return AS_MODULE(moduleVal);
    }

    ObjModule *module = ALLOCATE_OBJ(vm, ObjModule, OBJ_MODULE);
    initTable(&module->values);
    module->name = name;
    module->path = NULL;

    push(vm, OBJ_VAL(module));
    ObjString *__file__ = copyString(vm, "__file__", 8);
    push(vm, OBJ_VAL(__file__));

    tableSet(vm, &module->values, __file__, OBJ_VAL(name));
    tableSet(vm, &vm->modules, name, OBJ_VAL(module));

    pop(vm);
    pop(vm);

    return module;
}

ObjBoundMethod *newBoundMethod(DictuVM *vm, Value receiver, ObjClosure *method) {
    ObjBoundMethod *bound = ALLOCATE_OBJ(vm, ObjBoundMethod,
                                         OBJ_BOUND_METHOD);

    bound->receiver = receiver;
    bound->method = method;
    return bound;
}

ObjClass *newClass(DictuVM *vm, ObjString *name, ObjClass *superclass, ClassType type) {
    ObjClass *klass = ALLOCATE_OBJ(vm, ObjClass, OBJ_CLASS);
    klass->name = name;
    klass->superclass = superclass;
    klass->type = type;
    initTable(&klass->abstractMethods);
    initTable(&klass->privateMethods);
    initTable(&klass->publicMethods);
    initTable(&klass->variables);
    initTable(&klass->constants);
    klass->classAnnotations = NULL;
    klass->methodAnnotations = NULL;
    klass->fieldAnnotations = NULL;

    push(vm, OBJ_VAL(klass));
    ObjString *nameString = copyString(vm, "_name", 5);
    push(vm, OBJ_VAL(nameString));
    tableSet(vm, &klass->constants, nameString, OBJ_VAL(name));
    pop(vm);
    pop(vm);

    return klass;
}

ObjEnum *newEnum(DictuVM *vm, ObjString *name) {
    ObjEnum *enumObj = ALLOCATE_OBJ(vm, ObjEnum , OBJ_ENUM);
    enumObj->name = name;
    initTable(&enumObj->values);
    return enumObj;
}

ObjClosure *newClosure(DictuVM *vm, ObjFunction *function) {
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

ObjFunction *newFunction(DictuVM *vm, ObjModule *module, FunctionType type, AccessLevel level) {
    ObjFunction *function = ALLOCATE_OBJ(vm, ObjFunction, OBJ_FUNCTION);
    function->arity = 0;
    function->arityOptional = 0;
    function->isVariadic = 0;
    function->upvalueCount = 0;
    function->propertyCount = 0;
    function->propertyIndexes = NULL;
    function->propertyNames = NULL;
    function->privatePropertyCount = 0;
    function->privatePropertyIndexes = NULL;
    function->propertyNames = NULL;
    function->name = NULL;
    function->inlineCacheCount = 0;
    function->inlineCaches = NULL;
    function->type = type;
    function->accessLevel = level;
    function->module = module;
    initChunk(vm, &function->chunk);

    return function;
}

ObjInstance *newInstance(DictuVM *vm, ObjClass *klass) {
    ObjInstance *instance = ALLOCATE_OBJ(vm, ObjInstance, OBJ_INSTANCE);
    instance->klass = klass;
    initTable(&instance->publicAttributes);
    initTable(&instance->privateAttributes);

    push(vm, OBJ_VAL(instance));
    ObjString *classString = copyString(vm, "_class", 6);
    push(vm, OBJ_VAL(classString));
    tableSet(vm, &instance->publicAttributes, classString, OBJ_VAL(klass));
    pop(vm);
    pop(vm);

    return instance;
}

ObjNative *newNative(DictuVM *vm, NativeFn function) {
    ObjNative *native = ALLOCATE_OBJ(vm, ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}

static ObjString *allocateStringWithLen(DictuVM *vm, char *chars, int length,
                                 uint32_t hash, int character_len) {
    ObjString *string = ALLOCATE_OBJ(vm, ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;
    string->character_len = character_len;

    push(vm, OBJ_VAL(string));
    tableSet(vm, &vm->strings, string, NIL_VAL);
    pop(vm);
    return string;
}

static ObjString *allocateString(DictuVM *vm, char *chars, int length,
                                 uint32_t hash) {
    int character_len = utf8valid(chars) == 0 ? (int)utf8len(chars) : -1;
    return allocateStringWithLen(vm, chars, length, hash, character_len);
}

ObjList *newList(DictuVM *vm) {
    ObjList *list = ALLOCATE_OBJ(vm, ObjList, OBJ_LIST);
    initValueArray(&list->values);
    return list;
}

ObjDict *newDict(DictuVM *vm) {
    ObjDict *dict = ALLOCATE_OBJ(vm, ObjDict, OBJ_DICT);
    dict->count = 0;
    dict->activeCount = 0;
    dict->capacityMask = -1;
    dict->entries = NULL;
    return dict;
}

ObjSet *newSet(DictuVM *vm) {
    ObjSet *set = ALLOCATE_OBJ(vm, ObjSet, OBJ_SET);
    set->count = 0;
    set->capacityMask = -1;
    set->entries = NULL;
    return set;
}

ObjFile *newFile(DictuVM *vm) {
    return ALLOCATE_OBJ(vm, ObjFile, OBJ_FILE);
}

ObjAbstract *newAbstract(DictuVM *vm, AbstractFreeFn func, AbstractTypeFn type) {
    ObjAbstract *abstract = ALLOCATE_OBJ(vm, ObjAbstract, OBJ_ABSTRACT);
    abstract->data = NULL;
    abstract->func = func;
    abstract->type = type;
    abstract->grayFunc = NULL;
    abstract->excludeSelf = false;
    initTable(&abstract->values);

    return abstract;
}

ObjAbstract *newAbstractExcludeSelf(DictuVM *vm, AbstractFreeFn func, AbstractTypeFn type) {
    ObjAbstract *abstract = ALLOCATE_OBJ(vm, ObjAbstract, OBJ_ABSTRACT);
    abstract->data = NULL;
    abstract->func = func;
    abstract->type = type;
    abstract->grayFunc = NULL;
    abstract->excludeSelf = true;
    initTable(&abstract->values);

    return abstract;
}

ObjResult *newResult(DictuVM *vm, ResultStatus status, Value value) {
    ObjResult *result = ALLOCATE_OBJ(vm, ObjResult, OBJ_RESULT);
    result->status = status;
    result->value = value;

    return result;
}

Value newResultSuccess(DictuVM *vm, Value value) {
    push(vm, value);
    ObjResult *result = newResult(vm, SUCCESS, value);
    pop(vm);
    return OBJ_VAL(result);
}

Value newResultError(DictuVM *vm, char *errorMsg) {
    Value error = OBJ_VAL(copyString(vm, errorMsg, strlen(errorMsg)));
    push(vm, error);
    ObjResult *result = newResult(vm, ERR, error);
    pop(vm);
    return OBJ_VAL(result);
}

static uint32_t hashString(const char *key, int length) {
    uint32_t hash = 2166136261u;

    for (int i = 0; i < length; i++) {
        hash ^= key[i];
        hash *= 16777619;
    }

    return hash;
}

ObjString *takeString(DictuVM *vm, char *chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString *interned = tableFindString(&vm->strings, chars, length,
                                          hash);
    if (interned != NULL) {
        FREE_ARRAY(vm, char, chars, length + 1);
        return interned;
    }

    // Ensure terminating char is present
    chars[length] = '\0';
    return allocateString(vm, chars, length, hash);
}

ObjString *takeStringWithLen(DictuVM *vm, char *chars, int length, int character_len) {
    uint32_t hash = hashString(chars, length);
    ObjString *interned = tableFindString(&vm->strings, chars, length,
                                          hash);
    if (interned != NULL) {
        FREE_ARRAY(vm, char, chars, length + 1);
        return interned;
    }

    // Ensure terminating char is present
    chars[length] = '\0';
    return allocateStringWithLen(vm, chars, length, hash, character_len);
}

ObjString *copyString(DictuVM *vm, const char *chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString *interned = tableFindString(&vm->strings, chars, length,
                                          hash);
    if (interned != NULL) return interned;

    char *heapChars = ALLOCATE(vm, char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(vm, heapChars, length, hash);
}

ObjString *copyStringWithLen(DictuVM *vm, const char *chars, int length, int character_len) {
    uint32_t hash = hashString(chars, length);
    ObjString *interned = tableFindString(&vm->strings, chars, length,
                                          hash);
    if (interned != NULL) return interned;

    char *heapChars = ALLOCATE(vm, char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateStringWithLen(vm, heapChars, length, hash, character_len);
}

ObjUpvalue *newUpvalue(DictuVM *vm, Value *slot) {
    ObjUpvalue *upvalue = ALLOCATE_OBJ(vm, ObjUpvalue, OBJ_UPVALUE);
    upvalue->closed = NIL_VAL;
    upvalue->value = slot;
    upvalue->next = NULL;

    return upvalue;
}

char *listToString(DictuVM *vm, Value value, int *length) {
    int size = 50;
    ObjList *list = AS_LIST(value);
    char *listString = ALLOCATE(vm, char, size);
    memcpy(listString, "[", 1);
    int listStringLength = 1;

    for (int i = 0; i < list->values.count; ++i) {
        Value listValue = list->values.values[i];

        char *element;
        int elementSize;

        if (listValue == value) {
            element = "[...]";
            elementSize = 5;
        } else if (IS_STRING(listValue)) {
            ObjString *s = AS_STRING(listValue);
            element = s->chars;
            elementSize = s->length;
        } else {
            element = valueToString(vm, listValue, &elementSize);
        }

        if (elementSize > (size - listStringLength - 6)) {
            int oldSize = size;
            if (elementSize > size) {
                size = size + elementSize * 2 + 6;
            } else {
                size = size * 2 + 6;
            }

            listString = GROW_ARRAY(vm, listString, char, oldSize, size);
        }

        if (listValue == value) {
            memcpy(listString + listStringLength, element, elementSize);
            listStringLength += elementSize;
        } else if (IS_STRING(listValue)) {
            memcpy(listString + listStringLength, "\"", 1);
            memcpy(listString + listStringLength + 1, element, elementSize);
            memcpy(listString + listStringLength + 1 + elementSize, "\"", 1);
            listStringLength += elementSize + 2;
        } else {
            memcpy(listString + listStringLength, element, elementSize);
            listStringLength += elementSize;
            FREE_ARRAY(vm, char, element, elementSize + 1);
        }

        if (i != list->values.count - 1) {
            memcpy(listString + listStringLength, ", ", 2);
            listStringLength += 2;
        }
    }

    memcpy(listString + listStringLength, "]", 1);
    listString[listStringLength + 1] = '\0';

    if (listStringLength + 2 != size) {
        listString = SHRINK_ARRAY(vm, listString, char, size, listStringLength + 2);
    }

    *length = listStringLength + 1;
    return listString;
}

char *dictToString(DictuVM *vm, Value value, int *length) {
   int count = 0;
   int size = 50;
   ObjDict *dict = AS_DICT(value);
   char *dictString = ALLOCATE(vm, char, size);
   memcpy(dictString, "{", 1);
   int dictStringLength = 1;

   for (int i = 0; i <= dict->capacityMask; ++i) {
       DictItem *item = &dict->entries[i];
       if (IS_EMPTY(item->key)) {
           continue;
       }

       count++;

       char *key;
       int keySize;

       if (IS_STRING(item->key)) {
           ObjString *s = AS_STRING(item->key);
           key = s->chars;
           keySize = s->length;
       } else {
           key = valueToString(vm, item->key, &keySize);
       }

       if (keySize > (size - dictStringLength - keySize - 4)) {
           int oldSize = size;
           if (keySize > size) {
               size += keySize * 2 + 4;
           } else {
               size *= 2 + 4;
           }

           dictString = GROW_ARRAY(vm, dictString, char, oldSize, size);
       }

       if (IS_STRING(item->key)) {
           memcpy(dictString + dictStringLength, "\"", 1);
           memcpy(dictString + dictStringLength + 1, key, keySize);
           memcpy(dictString + dictStringLength + 1 + keySize, "\": ", 3);
           dictStringLength += 4 + keySize;
       } else {
           memcpy(dictString + dictStringLength, key, keySize);
           memcpy(dictString + dictStringLength + keySize, ": ", 2);
           dictStringLength += 2 + keySize;
           FREE_ARRAY(vm, char, key, keySize + 1);
       }

       char *element;
       int elementSize;
       if (item->value == value){
           element = "{...}";
           elementSize = 5;
       } else if (IS_STRING(item->value)) {
           ObjString *s = AS_STRING(item->value);
           element = s->chars;
           elementSize = s->length;
       } else {
           element = valueToString(vm, item->value, &elementSize);
       }

       if (elementSize > (size - dictStringLength - elementSize - 6)) {
           int oldSize = size;
           if (elementSize > size) {
               size += elementSize * 2 + 6;
           } else {
               size = size * 2 + 6;
           }

           dictString = GROW_ARRAY(vm, dictString, char, oldSize, size);
       }

       if (item->value == value) {
           memcpy(dictString + dictStringLength, element, elementSize);
           dictStringLength += elementSize;
       } else if (IS_STRING(item->value)) {
           memcpy(dictString + dictStringLength, "\"", 1);
           memcpy(dictString + dictStringLength + 1, element, elementSize);
           memcpy(dictString + dictStringLength + 1 + elementSize, "\"", 1);
           dictStringLength += 2 + elementSize;
       } else {
           memcpy(dictString + dictStringLength, element, elementSize);
           dictStringLength += elementSize;
           FREE_ARRAY(vm, char, element, elementSize + 1);
       }

       if (count != dict->count) {
           memcpy(dictString + dictStringLength, ", ", 2);
           dictStringLength += 2;
       }
   }

   memcpy(dictString + dictStringLength, "}", 1);
   dictString[dictStringLength + 1] = '\0';

   if (dictStringLength + 2 != size) {
       dictString = SHRINK_ARRAY(vm, dictString, char, size, dictStringLength + 2);
   }

   *length = dictStringLength + 1;
   return dictString;
}

char *setToString(DictuVM *vm, Value value, int *length) {
    int count = 0;
    int size = 50;
    ObjSet *set = AS_SET(value);
    char *setString = ALLOCATE(vm, char, size);
    memcpy(setString, "{", 1);
    int setStringLength = 1;

    for (int i = 0; i <= set->capacityMask; ++i) {
        SetItem *item = &set->entries[i];
        if (IS_EMPTY(item->value) || item->deleted)
            continue;

        count++;

        char *element;
        int elementSize;

        if (IS_STRING(item->value)) {
            ObjString *s = AS_STRING(item->value);
            element = s->chars;
            elementSize = s->length;
        } else {
            element = valueToString(vm, item->value, &elementSize);
        }

        if (elementSize > (size - setStringLength - 5)) {
            int oldSize = size;
            if (elementSize > size * 2) {
                size += elementSize * 2 + 5;
            } else {
                size = size * 2 + 5;
            }

            setString = GROW_ARRAY(vm, setString, char, oldSize, size);
        }


        if (IS_STRING(item->value)) {
            memcpy(setString + setStringLength, "\"", 1);
            memcpy(setString + setStringLength + 1, element, elementSize);
            memcpy(setString + setStringLength + 1 + elementSize, "\"", 1);
            setStringLength += 2 + elementSize;
        } else {
            memcpy(setString + setStringLength, element, elementSize);
            setStringLength += elementSize;
            FREE_ARRAY(vm, char, element, elementSize + 1);
        }

        if (count != set->count) {
            memcpy(setString + setStringLength, ", ", 2);
            setStringLength += 2;
        }
    }

    memcpy(setString + setStringLength, "}", 1);
    setString[setStringLength + 1] = '\0';

    if (setStringLength + 2 != size) {
        setString = SHRINK_ARRAY(vm, setString, char, size, setStringLength + 2);
    }

    *length = setStringLength + 1;
    return setString;
}

char *classToString(DictuVM *vm, Value value, int *length) {
    ObjClass *klass = AS_CLASS(value);
    char *classString = ALLOCATE(vm, char, klass->name->length + 7);
    memcpy(classString, "<Cls ", 5);
    memcpy(classString + 5, klass->name->chars, klass->name->length);
    memcpy(classString + 5 + klass->name->length, ">", 1);
    classString[klass->name->length + 6] = '\0';
    *length = klass->name->length + 6;
    return classString;
}

static bool listContains(ObjList *list, Value value) {
    for (int i = 0; i < list->values.count; ++i) {
        if (valuesEqual(list->values.values[i], value)) {
            return true;
        }
    }

    return false;
}

ObjDict *classToDict(DictuVM *vm, Value value) {
    ObjClass *klass = AS_CLASS(value);

    ObjDict *classDict = newDict(vm);
    push(vm, OBJ_VAL(classDict));

    ObjDict *variablesDict = newDict(vm);
    push(vm, OBJ_VAL(variablesDict));

    ObjDict *constantsDict = newDict(vm);
    push(vm, OBJ_VAL(constantsDict));

    ObjList *methodsList = newList(vm);
    push(vm, OBJ_VAL(methodsList));

    while (klass != NULL) {
        for (int i = 0; i < klass->variables.capacity; i++) {
            if (klass->variables.entries[i].key == NULL) {
                continue;
            }
            dictSet(vm, variablesDict, OBJ_VAL(klass->variables.entries[i].key), klass->variables.entries[i].value);
        }

        for (int i = 0; i < klass->constants.capacity; i++) {
            if (klass->constants.entries[i].key == NULL) {
                continue;
            }
            dictSet(vm, constantsDict, OBJ_VAL(klass->constants.entries[i].key), klass->constants.entries[i].value);
        }

        for (int i = 0; i < klass->publicMethods.capacity; i++) {
            if (klass->publicMethods.entries[i].key == NULL) {
                continue;
            }

            if (listContains(methodsList, OBJ_VAL(klass->publicMethods.entries[i].key))) {
                continue;
            }

            writeValueArray(vm, &methodsList->values, OBJ_VAL(klass->publicMethods.entries[i].key));
        }

        klass = klass->superclass;
    }

    Value variables = OBJ_VAL(copyString(vm, "public_variables", 16));
    push(vm, OBJ_VAL(variables));
    dictSet(vm, classDict, variables, OBJ_VAL(variablesDict));
    pop(vm);
    pop(vm); // variablesDict

    Value constants = OBJ_VAL(copyString(vm, "constants", 9));
    push(vm, OBJ_VAL(constants));
    dictSet(vm, classDict, constants, OBJ_VAL(constantsDict));
    pop(vm);
    pop(vm); // constantsDict

    Value methods = OBJ_VAL(copyString(vm, "public_methods", 14));
    push(vm, OBJ_VAL(methods));
    dictSet(vm, classDict, methods, OBJ_VAL(methodsList));
    pop(vm);
    pop(vm); // methodsList

    pop(vm); // classDict
    return classDict;
}

char *instanceToString(DictuVM *vm, Value value, int *length) {
    ObjInstance *instance = AS_INSTANCE(value);
    int len = instance->klass->name->length + 11;
    char *instanceString = ALLOCATE(vm, char, len + 1);
    memcpy(instanceString, "<", 1);
    memcpy(instanceString + 1, instance->klass->name->chars, instance->klass->name->length);
    memcpy(instanceString + 1 + instance->klass->name->length, " instance>", 10);
    instanceString[len] = '\0';
    *length = len;
    return instanceString;
}

char *objectToString(DictuVM *vm, Value value, int *length) {
    switch (OBJ_TYPE(value)) {
        case OBJ_MODULE: {
            ObjModule *module = AS_MODULE(value);
            int len = module->name->length + 9;
            char *moduleString = ALLOCATE(vm, char, len + 1);
            memcpy(moduleString, "<Module ", 8);
            memcpy(moduleString + 8, module->name->chars, module->name->length);
            memcpy(moduleString + 8 + module->name->length, ">", 1);
            moduleString[len] = '\0';
            *length = len;
            return moduleString;
        }

        case OBJ_CLASS: {
            if (IS_TRAIT(value)) {
                ObjClass *trait = AS_CLASS(value);
                int len = trait->name->length + 8;
                char *traitString = ALLOCATE(vm, char, len + 1);
                memcpy(traitString, "<Trait ", 7);
                memcpy(traitString + 7, trait->name->chars, trait->name->length);
                memcpy(traitString + 7 + trait->name->length, ">", 1);
                traitString[len] = '\0';
                *length = len;
                return traitString;
            }

            return classToString(vm, value, length);
        }

        case OBJ_ENUM: {
            ObjEnum *enumObj = AS_ENUM(value);
            int len = enumObj->name->length + 7;
            char *enumString = ALLOCATE(vm, char, len + 1);
            memcpy(enumString, "<Enum ", 6);
            memcpy(enumString + 6, enumObj->name->chars, enumObj->name->length);
            memcpy(enumString + 6 + enumObj->name->length, ">", 1);
            enumString[len] = '\0';
            *length = len;
            return enumString;
        }

        case OBJ_BOUND_METHOD: {
            ObjBoundMethod *method = AS_BOUND_METHOD(value);
            char *methodString;
            int len;

            if (method->method->function->name != NULL) {
                switch (method->method->function->type) {
                    case TYPE_STATIC: {
                        len = method->method->function->name->length + 16;
                        methodString = ALLOCATE(vm, char, len + 1);
                        memcpy(methodString, "<Static Method ", 15);
                        memcpy(methodString + 15, method->method->function->name->chars, method->method->function->name->length);
                        memcpy(methodString + 15 + method->method->function->name->length, ">", 1);
                        methodString[len] = '\0';
                        break;
                    }

                    default: {
                        len = method->method->function->name->length + 15;
                        methodString = ALLOCATE(vm, char, len + 1);
                        memcpy(methodString, "<Bound Method ", 14);
                        memcpy(methodString + 14, method->method->function->name->chars, method->method->function->name->length);
                        memcpy(methodString + 14 + method->method->function->name->length, ">", 1);
                        methodString[len] = '\0';
                        break;
                    }
                }
            } else {
                switch (method->method->function->type) {
                    case TYPE_STATIC: {
                        len = 15;
                        methodString = ALLOCATE(vm, char, len + 1);
                        memcpy(methodString, "<Static Method>", len);
                        methodString[len] = '\0';
                        break;
                    }

                    default: {
                        len = 14;
                        methodString = ALLOCATE(vm, char, len + 1);
                        memcpy(methodString, "<Bound Method>", len);
                        methodString[len] = '\0';
                        break;
                    }
                }
            }

            *length = len;
            return methodString;
        }

        case OBJ_CLOSURE: {
            ObjClosure *closure = AS_CLOSURE(value);
            char *closureString;
            int len;

            if (closure->function->name != NULL) {
                len = closure->function->name->length + 5;
                closureString = ALLOCATE(vm, char, len + 1);
                memcpy(closureString, "<fn ", 4);
                memcpy(closureString + 4, closure->function->name->chars, closure->function->name->length);
                memcpy(closureString + 4 + closure->function->name->length, ">", 1);
                closureString[len] = '\0';
            } else {
                len = 8;
                closureString = ALLOCATE(vm, char, len + 1);
                memcpy(closureString, "<Script>", len);
                closureString[len] = '\0';
            }

            *length = len;
            return closureString;
        }

        case OBJ_FUNCTION: {
            ObjFunction *function = AS_FUNCTION(value);
            char *functionString;
            int len;

            if (function->name != NULL) {
                len = function->name->length + 5;
                functionString = ALLOCATE(vm, char, len + 1);
                memcpy(functionString, "<fn ", 4);
                memcpy(functionString + 4, function->name->chars, function->name->length);
                memcpy(functionString + 4 + function->name->length, ">", 1);
                functionString[len] = '\0';
            } else {
                len = 4;
                functionString = ALLOCATE(vm, char, len + 1);
                memcpy(functionString, "<fn>", len);
                functionString[len] = '\0';
            }

            *length = len;
            return functionString;
        }

        case OBJ_INSTANCE: {
            return instanceToString(vm, value, length);
        }

        case OBJ_NATIVE: {
            int len = 11;
            char *nativeString = ALLOCATE(vm, char, len + 1);
            memcpy(nativeString, "<fn native>", len);
            nativeString[len] = '\0';
            *length = len;
            return nativeString;
        }

        case OBJ_STRING: {
            ObjString *stringObj = AS_STRING(value);
            char *string = ALLOCATE(vm, char, stringObj->length + 1);
            memcpy(string, stringObj->chars, stringObj->length);
            string[stringObj->length] = '\0';
            *length = stringObj->length;
            return string;
        }

        case OBJ_FILE: {
            ObjFile *file = AS_FILE(value);
            int pathLen = strlen(file->path);
            int len = pathLen + 7;
            char *fileString = ALLOCATE(vm, char, len + 1);
            memcpy(fileString, "<File ", 6);
            memcpy(fileString + 6, file->path, pathLen);
            memcpy(fileString + 6 + pathLen, ">", 1);
            fileString[len] = '\0';
            *length = len;
            return fileString;
        }

        case OBJ_LIST: {
            return listToString(vm, value, length);
        }

        case OBJ_DICT: {
            return dictToString(vm, value, length);
        }

        case OBJ_SET: {
            return setToString(vm, value, length);
        }

        case OBJ_UPVALUE: {
            int len = 7;
            char *upvalueString = ALLOCATE(vm, char, len + 1);
            memcpy(upvalueString, "upvalue", len);
            upvalueString[len] = '\0';
            *length = len;
            return upvalueString;
        }

        case OBJ_ABSTRACT: {
            ObjAbstract *abstract = AS_ABSTRACT(value);
            return abstract->type(vm, abstract, length);
        }

        case OBJ_RESULT: {
            ObjResult *result = AS_RESULT(value);
            if (result->status == SUCCESS) {
                int len = 12;
                char *resultString = ALLOCATE(vm, char, len + 1);
                memcpy(resultString, "<Result Suc>", len);
                resultString[len] = '\0';
                *length = len;
                return resultString;
            }

            int len = 12;
            char *resultString = ALLOCATE(vm, char, len + 1);
            memcpy(resultString, "<Result Err>", len);
            resultString[len] = '\0';
            *length = len;
            return resultString;
        }
    }

    int len = 7;
    char *unknown = ALLOCATE(vm, char, len + 1);
    memcpy(unknown, "unknown", len);
    unknown[len] = '\0';
    *length = len;
    return unknown;
}
