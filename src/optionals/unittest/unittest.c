#include "unittest.h"

#include "unittest-source.h"

Value createUnitTestModule(DictuVM *vm) {
    ObjClosure *module = compileModuleToClosure(vm, "UnitTest", DICTU_UNITTEST_SOURCE);

    if (module == NULL) {
        return EMPTY_VAL;
    }

    return OBJ_VAL(module);
}
