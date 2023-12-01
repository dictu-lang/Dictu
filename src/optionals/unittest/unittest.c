#include "unittest.h"

#include "unittest-source.h"

static Value mockNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount > 2) {
        runtimeError(vm, "mock() takes 1 or 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_CLASS(args[0])) {
        runtimeError(vm, "First argument passed to mock() must be a class");
        return EMPTY_VAL;
    }

    ObjClass *klass = AS_CLASS(args[0]);
    ObjDict *returnValues = NULL;

    if (argCount == 2) {
        if (!IS_DICT(args[1])) {
            runtimeError(vm, "Second argument passed to mock() must be a dictionary");
            return EMPTY_VAL;
        }

        returnValues = AS_DICT(args[1]);
    }

    ObjString *mockString = copyString(vm, "Mock", 4);
    push(vm, OBJ_VAL(mockString));
    ObjClass *mockedClass = newClass(vm, mockString, NULL, CLASS_DEFAULT);
    pop(vm);
    push(vm, OBJ_VAL(mockedClass));

    for (int i = 0; i < klass->publicMethods.capacity; ++i) {
        if (klass->publicMethods.entries[i].key == NULL) {
            continue;
        }

        ObjFunction *emptyFunc = newFunction(vm, vm->frames[vm->frameCount - 1].closure->function->module, TYPE_METHOD, ACCESS_PUBLIC);
        push(vm, OBJ_VAL(emptyFunc));

        emptyFunc->name = klass->publicMethods.entries[i].key;

        Value val;
        if (returnValues != NULL && dictGet(returnValues, OBJ_VAL(klass->publicMethods.entries[i].key), &val)) {
            int constant = addConstant(vm, &emptyFunc->chunk, val);
            writeChunk(vm, &emptyFunc->chunk, constant, 1);
            writeChunk(vm, &emptyFunc->chunk, OP_CONSTANT, 1);
        } else {
            writeChunk(vm, &emptyFunc->chunk, OP_NIL, 1);
        }

        writeChunk(vm, &emptyFunc->chunk, OP_RETURN, 1);

        ObjClosure *closure = newClosure(vm, emptyFunc);
        pop(vm); // function
        push(vm, OBJ_VAL(closure));

        tableSet(
            vm,
            &mockedClass->publicMethods, klass->publicMethods.entries[i].key,
            OBJ_VAL(closure)
        );
        pop(vm); // closure
    }

    ObjInstance *mockInstance = newInstance(vm, mockedClass);
    pop(vm); // mock

    return OBJ_VAL(mockInstance);
}

Value createUnitTestModule(DictuVM *vm) {
    ObjClosure *closure = compileModuleToClosure(vm, "UnitTest", DICTU_UNITTEST_SOURCE);

    if (closure == NULL) {
        return EMPTY_VAL;
    }

    push(vm, OBJ_VAL(closure));
    defineNative(vm, &closure->function->module->values, "mock", mockNative);
    pop(vm);

    return OBJ_VAL(closure);
}
