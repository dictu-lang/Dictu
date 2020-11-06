#include "optionals.h"

BuiltinModules modules[] = {
        {"JSON", &createJSONModule},
        {"Path", &createPathModule},
#ifndef DISABLE_HTTP
        {"HTTP", &createHTTPModule},
#endif
        {"Datetime", &createDatetimeModule},
        {"Random", &createRandomModule},
        {"Env", &createEnvModule},
        {"Socket", &createSocketModule},
        {"Process", &createProcessModule},
        {"Math", &createMathsModule},
        {NULL, NULL}
};

ObjModule *importBuiltinModule(VM *vm, int index) {
    return modules[index].module(vm);
}

Value getErrno(VM* vm, ObjModule* module) {
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
