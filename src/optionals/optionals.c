#include "optionals.h"

BuiltinModules modules[] = {
        {"Math", &createMathsModule, false},
        {"Env", &createEnvModule, false},
        {"JSON", &createJSONModule, false},
        {"Path", &createPathModule, false},
        {"Datetime", &createDatetimeModule, false},
        {"Socket", &createSocketModule, false},
        {"Random", &createRandomModule, false},
        {"Base64", &createBase64Module, false},
        {"Hashlib", &createHashlibModule, false},
        {"Sqlite", &createSqliteModule, false},
        {"Process", &createProcessModule, false},
        {"UnitTest", &createUnitTestModule, true},
        {"Inspect", &createInspectModule, false},
#ifndef DISABLE_HTTP
        {"HTTP", &createHTTPModule, false},
#endif
        {NULL, NULL, false}
};

Value importBuiltinModule(DictuVM *vm, int index) {
    return modules[index].module(vm);
}

int findBuiltinModule(char *name, int length, bool *dictuSource) {
    for (int i = 0; modules[i].module != NULL; ++i) {
        if (strncmp(modules[i].name, name, length) == 0) {
            *dictuSource = modules[i].dictuSource;

            return i;
        }
    }

    return -1;
}
