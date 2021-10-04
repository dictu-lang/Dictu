#include "functools.h"

#include "functools-source.h"

Value createFuncToolsModule(DictuVM *vm) {
    ObjClosure *module = compileModuleToClosure(vm, "FuncTools", DICTU_FUNCTOOLS_SOURCE);

    return OBJ_VAL(module);
}
