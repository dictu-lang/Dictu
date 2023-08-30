#include "argparse.h"

#include "argparse-source.h"

Value createArgParseModule(DictuVM *vm) {
    ObjClosure *closure = compileModuleToClosure(vm, "Argparse", DICTU_ARGPARSE_SOURCE);

    if (closure == NULL) {
        return EMPTY_VAL;
    }

    return OBJ_VAL(closure);
}
