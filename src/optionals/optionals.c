#include "optionals.h"

BuiltinModules modules[] = {
        {"Math", &createMathsModule},
        {"Env", &createEnvModule},
        {"JSON", &createJSONModule},
        {"Path", &createPathModule},
        {"Datetime", &createDatetimeModule},
        {"Socket", &createSocketModule},
        {"Random", &createRandomModule},
#ifndef DISABLE_HTTP
        {"HTTP", &createHTTPModule},
#endif
        {NULL, NULL}
};

ObjModule *importBuiltinModule(DictuVM *vm, int index) {
    return modules[index].module(vm);
}

Value getErrno(DictuVM* vm, ObjModule* module) {
    Value errno_value = 0;
    ObjString *name = copyString(vm, "errno", 5);
    tableGet(&module->values, name, &errno_value);
    return errno_value;
}

int findBuiltinModule(char *name, int length) {
    for (int i = 0; modules[i].module != NULL; ++i) {
        if (strncmp(modules[i].name, name, length) == 0) {
            return i;
        }
    }

    return -1;
}
