#include "optionals.h"

BuiltinModules modules[] = {
        {"Math", &createMathsModule},
        {"Env", &createEnvModule},
        {"JSON", &createJSONModule},
        {"Path", &createPathModule},
        {"Datetime", &createDatetimeModule},
        {"Socket", &createSocketModule},
        {"Random", &createRandomModule},
        {"Base64", &createBase64Module},
        {"Hashlib", &createHashlibModule},
        {"Sqlite", &createSqliteModule},
        {"Process", &createProcessModule},
#ifndef DISABLE_HTTP
        {"HTTP", &createHTTPModule},
#endif
        {NULL, NULL}
};

ObjModule *importBuiltinModule(DictuVM *vm, int index) {
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
