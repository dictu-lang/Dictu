#include <string.h>

#include "archiver.h"
#include "optionals.h"
#include "../vm/vm.h"

Value createArchiveModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Archive", 7);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Archive methods
     */
    // defineNative(vm, &module->values, "print", printLog);

    defineNativeProperty(vm, &module->values, "ZIP", OBJ_VAL(copyString(vm, "ZIP", strlen("ZIP"))));


    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
