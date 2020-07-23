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

ObjModule *importBuiltinModule(VM *vm, char *name) {
    for (int i = 0; modules[i].module != NULL; ++i) {
        if (strcmp(modules[i].name, name) == 0) {
            return modules[i].module(vm);
        }
    }

    return NULL;
}
