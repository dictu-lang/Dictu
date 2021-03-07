#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "memory.h"
#include "vm.h"

#ifdef DEBUG_TRACE_GC
#include <stdio.h>
#include "debug.h"
#endif

#define GC_HEAP_GROW_FACTOR 2

void *reallocate(DictuVM *vm, void *previous, size_t oldSize, size_t newSize) {
    vm->bytesAllocated += newSize - oldSize;

#ifdef DEBUG_TRACE_MEM
    printf("Total memory usage: %zu\nNew allocation: %zu\nOld allocation: %zu\n\n", vm->bytesAllocated, newSize, oldSize);
#endif

    if (newSize > oldSize) {
#ifdef DEBUG_STRESS_GC
        collectGarbage(vm);
#endif

        if (vm->bytesAllocated > vm->nextGC) {
            collectGarbage(vm);
        }
    }

    if (newSize == 0) {
        free(previous);
        return NULL;
    }

    return realloc(previous, newSize);
}

void grayObject(DictuVM *vm, Obj *object) {
    if (object == NULL) return;

    // Don't get caught in cycle.
    if (object->isDark) return;

#ifdef DEBUG_TRACE_GC
    printf("%p gray ", (void *)object);
    printValue(OBJ_VAL(object));
    printf("\n");
#endif

    object->isDark = true;

    if (vm->grayCapacity < vm->grayCount + 1) {
        vm->grayCapacity = GROW_CAPACITY(vm->grayCapacity);

        // Not using reallocate() here because we don't want to trigger the
        // GC inside a GC!
        vm->grayStack = realloc(vm->grayStack,
                               sizeof(Obj *) * vm->grayCapacity);
    }

    vm->grayStack[vm->grayCount++] = object;
}

void grayValue(DictuVM *vm, Value value) {
    if (!IS_OBJ(value)) return;
    grayObject(vm, AS_OBJ(value));
}

static void grayArray(DictuVM *vm, ValueArray *array) {
    for (int i = 0; i < array->count; i++) {
        grayValue(vm, array->values[i]);
    }
}

static void blackenObject(DictuVM *vm, Obj *object) {
#ifdef DEBUG_TRACE_GC
    printf("%p blacken ", (void *)object);
    printValue(OBJ_VAL(object));
    printf("\n");
#endif

    switch (object->type) {
        case OBJ_MODULE: {
            ObjModule *module = (ObjModule *) object;
            grayObject(vm, (Obj *) module->name);
            grayObject(vm, (Obj *) module->path);
            grayTable(vm, &module->values);
            break;
        }

        case OBJ_BOUND_METHOD: {
            ObjBoundMethod *bound = (ObjBoundMethod *) object;
            grayValue(vm, bound->receiver);
            grayObject(vm, (Obj *) bound->method);
            break;
        }

        case OBJ_CLASS: {
            ObjClass *klass = (ObjClass *) object;
            grayObject(vm, (Obj *) klass->name);
            grayObject(vm, (Obj *) klass->superclass);
            grayTable(vm, &klass->publicMethods);
            grayTable(vm, &klass->privateMethods);
            grayTable(vm, &klass->abstractMethods);
            grayTable(vm, &klass->publicProperties);
            break;
        }

        case OBJ_ENUM: {
            ObjEnum *enumObj = (ObjEnum *) object;
            grayObject(vm, (Obj *) enumObj->name);
            grayTable(vm, &enumObj->values);
            break;
        }

        case OBJ_CLOSURE: {
            ObjClosure *closure = (ObjClosure *) object;
            grayObject(vm, (Obj *) closure->function);
            for (int i = 0; i < closure->upvalueCount; i++) {
                grayObject(vm, (Obj *) closure->upvalues[i]);
            }
            break;
        }

        case OBJ_FUNCTION: {
            ObjFunction *function = (ObjFunction *) object;
            grayObject(vm, (Obj *) function->name);
            grayArray(vm, &function->chunk.constants);
            break;
        }

        case OBJ_INSTANCE: {
            ObjInstance *instance = (ObjInstance *) object;
            grayObject(vm, (Obj *) instance->klass);
            grayTable(vm, &instance->publicFields);
            grayTable(vm, &instance->privateFields);
            break;
        }

        case OBJ_UPVALUE:
            grayValue(vm, ((ObjUpvalue *) object)->closed);
            break;

        case OBJ_LIST: {
            ObjList *list = (ObjList *) object;
            grayArray(vm, &list->values);
            break;
        }

        case OBJ_DICT: {
            ObjDict *dict = (ObjDict *) object;
            grayDict(vm, dict);
            break;
        }

        case OBJ_SET: {
            ObjSet *set = (ObjSet *) object;
            graySet(vm, set);
            break;
        }

        case OBJ_ABSTRACT: {
            ObjAbstract *abstract = (ObjAbstract *) object;
            grayTable(vm, &abstract->values);
            break;
        }

        case OBJ_RESULT: {
            ObjResult *result = (ObjResult *) object;
            grayValue(vm, result->value);
            break;
        }

        case OBJ_NATIVE:
        case OBJ_STRING:
        case OBJ_FILE:
            break;
    }
}

void freeObject(DictuVM *vm, Obj *object) {
#ifdef DEBUG_TRACE_GC
    printf("%p free type %d\n", (void*)object, object->type);
#endif

    switch (object->type) {
        case OBJ_MODULE: {
            ObjModule *module = (ObjModule *) object;
            freeTable(vm, &module->values);
            FREE(vm, ObjModule, object);
            break;
        }

        case OBJ_BOUND_METHOD: {
            FREE(vm, ObjBoundMethod, object);
            break;
        }

        case OBJ_CLASS: {
            ObjClass *klass = (ObjClass *) object;
            freeTable(vm, &klass->publicMethods);
            freeTable(vm, &klass->privateMethods);
            freeTable(vm, &klass->abstractMethods);
            freeTable(vm, &klass->publicProperties);
            FREE(vm, ObjClass, object);
            break;
        }

        case OBJ_ENUM: {
            ObjEnum *enumObj = (ObjEnum *) object;
            freeTable(vm, &enumObj->values);
            FREE(vm, ObjEnum, object);
            break;
        }

        case OBJ_CLOSURE: {
            ObjClosure *closure = (ObjClosure *) object;
            FREE_ARRAY(vm, ObjUpvalue*, closure->upvalues, closure->upvalueCount);
            FREE(vm, ObjClosure, object);
            break;
        }

        case OBJ_FUNCTION: {
            ObjFunction *function = (ObjFunction *) object;
            if (function->type == TYPE_INITIALIZER) {
                if (function->privatePropertyCount > 0) {
                    FREE_ARRAY(vm, int, function->privatePropertyNames, function->privatePropertyCount);
                    FREE_ARRAY(vm, int, function->privatePropertyIndexes, function->privatePropertyCount);
                }
                if (function->propertyCount > 0) {
                    FREE_ARRAY(vm, int, function->propertyNames, function->propertyCount);
                    FREE_ARRAY(vm, int, function->propertyIndexes, function->propertyCount);
                }
            }
            freeChunk(vm, &function->chunk);
            FREE(vm, ObjFunction, object);
            break;
        }

        case OBJ_INSTANCE: {
            ObjInstance *instance = (ObjInstance *) object;
            freeTable(vm, &instance->publicFields);
            freeTable(vm, &instance->privateFields);
            FREE(vm, ObjInstance, object);
            break;
        }

        case OBJ_NATIVE: {
            FREE(vm, ObjNative, object);
            break;
        }

        case OBJ_STRING: {
            ObjString *string = (ObjString *) object;
            FREE_ARRAY(vm, char, string->chars, string->length + 1);
            FREE(vm, ObjString, object);
            break;
        }

        case OBJ_LIST: {
            ObjList *list = (ObjList *) object;
            freeValueArray(vm, &list->values);
            FREE(vm, ObjList, list);
            break;
        }

        case OBJ_DICT: {
            ObjDict *dict = (ObjDict *) object;
            FREE_ARRAY(vm, DictItem, dict->entries, dict->capacityMask + 1);
            FREE(vm, ObjDict, dict);
            break;
        }

        case OBJ_SET: {
            ObjSet *set = (ObjSet *) object;
            FREE_ARRAY(vm, SetItem, set->entries, set->capacityMask + 1);
            FREE(vm, ObjSet, set);
            break;
        }

        case OBJ_FILE: {
            FREE(vm, ObjFile, object);
            break;
        }

        case OBJ_UPVALUE: {
            FREE(vm, ObjUpvalue, object);
            break;
        }

        case OBJ_ABSTRACT: {
            ObjAbstract *abstract = (ObjAbstract*) object;
            abstract->func(vm, abstract);
            freeTable(vm, &abstract->values);
            FREE(vm, ObjAbstract, object);
            break;
        }

        case OBJ_RESULT: {
            FREE(vm, ObjResult, object);
            break;
        }
    }
}

void collectGarbage(DictuVM *vm) {
#ifdef DEBUG_TRACE_GC
    printf("-- gc begin\n");
    size_t before = vm->bytesAllocated;
#endif

    // Mark the stack roots.
    for (Value *slot = vm->stack; slot < vm->stackTop; slot++) {
        grayValue(vm, *slot);
    }

    for (int i = 0; i < vm->frameCount; i++) {
        grayObject(vm, (Obj *) vm->frames[i].closure);
    }

    // Mark the open upvalues.
    for (ObjUpvalue *upvalue = vm->openUpvalues;
         upvalue != NULL;
         upvalue = upvalue->next) {
        grayObject(vm, (Obj *) upvalue);
    }

    // Mark the global roots.
    grayTable(vm, &vm->modules);
    grayTable(vm, &vm->globals);
    grayTable(vm, &vm->numberMethods);
    grayTable(vm, &vm->boolMethods);
    grayTable(vm, &vm->nilMethods);
    grayTable(vm, &vm->stringMethods);
    grayTable(vm, &vm->listMethods);
    grayTable(vm, &vm->dictMethods);
    grayTable(vm, &vm->setMethods);
    grayTable(vm, &vm->fileMethods);
    grayTable(vm, &vm->classMethods);
    grayTable(vm, &vm->instanceMethods);
    grayTable(vm, &vm->resultMethods);
    grayCompilerRoots(vm);
    grayObject(vm, (Obj *) vm->initString);
    grayObject(vm, (Obj *) vm->replVar);

    // Traverse the references.
    while (vm->grayCount > 0) {
        // Pop an item from the gray stack.
        Obj *object = vm->grayStack[--vm->grayCount];
        blackenObject(vm, object);
    }

    // Delete unused interned strings.
    tableRemoveWhite(vm, &vm->strings);

    // Collect the white objects.
    Obj **object = &vm->objects;
    while (*object != NULL) {
        if (!((*object)->isDark)) {
            // This object wasn't reached, so remove it from the list and
            // free it.
            Obj *unreached = *object;
            *object = unreached->next;
            freeObject(vm, unreached);
        } else {
            // This object was reached, so unmark it (for the next GC) and
            // move on to the next.
            (*object)->isDark = false;
            object = &(*object)->next;
        }
    }

    // Adjust the heap size based on live memory.
    vm->nextGC = vm->bytesAllocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_TRACE_GC
    printf("-- gc collected %ld bytes (from %ld to %ld) next at %ld\n",
           before - vm->bytesAllocated, before, vm->bytesAllocated,
           vm->nextGC);
#endif
}

void freeObjects(DictuVM *vm) {
    Obj *object = vm->objects;
    while (object != NULL) {
        Obj *next = object->next;
        freeObject(vm, object);
        object = next;
    }

    free(vm->grayStack);
}
