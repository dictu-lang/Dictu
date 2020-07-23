#include "optionals.h"

BuiltinModules modules[] = {
        {"Math", &createMathsClass},
        {"Env", &createEnvClass},
        {"JSON", &createJSONClass},
        {"Path", &createPathClass},
        {"Datetime", &createDatetimeClass},
#ifndef DISABLE_HTTP
        {"HTTP", &createHTTPClass},
#endif
        {NULL, NULL}
};

ObjModule *importBuiltinModule(VM *vm, int index) {
    return modules[index].module(vm);
}

int findBuiltinModule(char *name, int length) {
    for (int i = 0; modules[i].module != NULL; ++i) {
        if (strncmp(modules[i].name, name, length) == 0) {
            return i;
        }
    }

    return -1;
}
