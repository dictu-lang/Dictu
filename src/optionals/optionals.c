#include "optionals.h"

BuiltinModules modules[] = {
    {"Argparse", &createArgParseModule, false},
    {"Math", &createMathsModule, false},
    {"Env", &createEnvModule, true},
    {"JSON", &createJSONModule, false},
    {"Log", &createLogModule, false},
    {"Path", &createPathModule, false},
    {"Datetime", &createDatetimeModule, false},
    {"Socket", &createSocketModule, false},
    {"Random", &createRandomModule, false},
    {"Base64", &createBase64Module, false},
    {"Hashlib", &createHashlibModule, false},
    {"Sqlite", &createSqliteModule, false},
    {"Process", &createProcessModule, false},
    {"System", &createSystemModule, false},
    {"Term", &createTermModule, false},
#ifndef DISABLE_UUID
    {"UUID", &createUuidModule, false},
#endif
    {"UnitTest", &createUnitTestModule, true},
    {"Inspect", &createInspectModule, false},
    {"Object", &createObjectModule, true},
#ifndef DISABLE_HTTP
    {"HTTP", &createHTTPModule, true},
#endif
#ifndef DISABLE_BIGINT
    {"BigInt", &createBigIntModule, false},
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
