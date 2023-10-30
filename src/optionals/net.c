#ifdef _WIN32
#include "windowsapi.h"
//#include <Ws2tcpip.h>
#endif

#include <arpa/inet.h>
#include <string.h>

#include "net.h"
#include "../vm/vm.h"

#define IP4_LEN 4
#define IP6_LEN 16

static Value parseIp4(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "parseIp4() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "parseIp4() argument must be a string");
        return EMPTY_VAL;
    }

    char *ipStr = AS_CSTRING(args[0]);
    char ignore[4] = {0};
    
#ifdef _WIN32
    if (InetPton(AF_INET, ipStr, ignore) == 0) {
        return newResultError(vm, "invalid ip4 address");
    }
#else
    if (inet_pton(AF_INET, ipStr, ignore) == 0) {
        return newResultError(vm, "invalid ip4 address");
    }
#endif

    unsigned char value[IP4_LEN] = {0};
    size_t index = 0;

    while (*ipStr) {
        if (isdigit((unsigned char)*ipStr)) {
            value[index] *= 10;
            value[index] += *ipStr - '0';
        } else {
            index++;
        }
        ipStr++;
    }

    ObjList *list = newList(vm);
    push(vm, OBJ_VAL(list));

    for (int i = 0; i < IP4_LEN; i++) {
        writeValueArray(vm, &list->values, NUMBER_VAL(value[i]));
    }

    pop(vm);

    return newResultSuccess(vm, OBJ_VAL(list));
}

Value createNetModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Net", 3);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    defineNativeProperty(vm, &module->values, "ip4Len", NUMBER_VAL(IP4_LEN));
    defineNativeProperty(vm, &module->values, "ip6Len", NUMBER_VAL(IP6_LEN));

    /**
     * Define Net methods
     */
    defineNative(vm, &module->values, "parseIp4", parseIp4);

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}

