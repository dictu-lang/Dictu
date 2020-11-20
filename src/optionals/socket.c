#include "socket.h"

#include <stdio.h>

#ifdef _WIN32
#include "windowsapi.h"
#include <io.h>
#define setsockopt(S, LEVEL, OPTNAME, OPTVAL, OPTLEN) setsockopt(S, LEVEL, OPTNAME, (char*)(OPTVAL), OPTLEN)

#ifndef __MINGW32__
// Fixes deprecation warning
unsigned long inet_addr_new(const char* cp) {
    unsigned long S_addr;
    inet_pton(AF_INET, cp, &S_addr);
    return S_addr;
}
#define inet_addr(cp) inet_addr_new(cp)
#endif

#define write(fd, buffer, count) _write(fd, buffer, count)
#define close(fd) closesocket(fd)
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

static Value createSocket(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "create() takes 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1])) {
        runtimeError(vm, "create() arguments must be a numbers");
        return EMPTY_VAL;
    }

    int socketFamily = AS_NUMBER(args[0]);
    int socketType = AS_NUMBER(args[1]);

    int sock = socket(socketFamily, socketType, 0);
    if (sock == -1) {
        SET_ERRNO(GET_SELF_CLASS);
        return NIL_VAL;
    }

    ObjSocket *s = newSocket(vm, sock, socketFamily, socketType, 0);
    return OBJ_VAL(s);
}

static Value bindSocket(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "bind() takes 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "host passed to bind() must be a string");
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[2])) {
        runtimeError(vm, "port passed to bind() must be a number");
        return EMPTY_VAL;
    }

    ObjSocket *sock = AS_SOCKET(args[0]);
    char *host = AS_CSTRING(args[1]);
    int port = AS_NUMBER(args[2]);

    struct sockaddr_in server;

    server.sin_family = sock->socketFamily;
    server.sin_addr.s_addr = inet_addr(host);
    server.sin_port = htons(port);

    if (bind(sock->socket, (struct sockaddr *)&server , sizeof(server)) < 0) {
        Value module = 0;
        tableGet(&vm->modules, copyString(vm, "Socket", 6), &module);
        SET_ERRNO(AS_MODULE(module));
        return NIL_VAL;
    }

    return TRUE_VAL;
}

static Value listenSocket(DictuVM *vm, int argCount, Value *args) {
    if (argCount > 1) {
        runtimeError(vm, "listen() takes 0 or 1 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    int backlog = SOMAXCONN;

    if (argCount == 1) {
        if (!IS_NUMBER(args[1])) {
            runtimeError(vm, "listen() argument must be a number");
            return EMPTY_VAL;
        }

        backlog = AS_NUMBER(args[1]);
    }

    ObjSocket *sock = AS_SOCKET(args[0]);
    if (listen(sock->socket, backlog) == -1) {
        Value module = 0;
        tableGet(&vm->modules, copyString(vm, "Socket", 6), &module);
        SET_ERRNO(AS_MODULE(module));
        return NIL_VAL;
    }

    return TRUE_VAL;
}

static Value acceptSocket(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "accept() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjSocket *sock = AS_SOCKET(args[0]);

    struct sockaddr_in client;
    int c = sizeof(struct sockaddr_in);
    int newSockId = accept(sock->socket, (struct sockaddr *)&client, (socklen_t*)&c);

    ObjList *list = initList(vm);
    push(vm, OBJ_VAL(list));

    ObjSocket *newSock = newSocket(vm, newSockId, sock->socketFamily, sock->socketProtocol, 0);

    push(vm, OBJ_VAL(newSock));
    writeValueArray(vm, &list->values, OBJ_VAL(newSock));
    pop(vm);

    // IPv6 is 39 chars
    char ip[40];
    inet_ntop(sock->socketFamily, &client.sin_addr, ip, 40);
    ObjString *string = copyString(vm, ip, strlen(ip));

    push(vm, OBJ_VAL(string));
    writeValueArray(vm, &list->values, OBJ_VAL(string));
    pop(vm);

    pop(vm);

    return OBJ_VAL(list);
}

static Value writeSocket(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "write() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "write() argument must be a string");
        return EMPTY_VAL;
    }

    ObjSocket *sock = AS_SOCKET(args[0]);
    ObjString *message = AS_STRING(args[1]);

    int writeRet = write(sock->socket , message->chars, message->length);

    if (writeRet == -1) {
        Value module = 0;
        tableGet(&vm->modules, copyString(vm, "Socket", 6), &module);
        SET_ERRNO(AS_MODULE(module));
        return NIL_VAL;
    }

    return NUMBER_VAL(writeRet);
}

static Value recvSocket(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "recv() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm, "recv() argument must be a number");
        return EMPTY_VAL;
    }

    ObjSocket *sock = AS_SOCKET(args[0]);
    int bufferSize = AS_NUMBER(args[1]);

    if (bufferSize < 1) {
        runtimeError(vm, "recv() argument must be greater than 1");
        return EMPTY_VAL;
    }

    char *buffer = ALLOCATE(vm, char, bufferSize);
    int readSize = recv(sock->socket, buffer, bufferSize, 0);

    if (readSize == -1) {
        Value module = 0;
        tableGet(&vm->modules, copyString(vm, "Socket", 6), &module);
        SET_ERRNO(AS_MODULE(module));
        FREE_ARRAY(vm, char, buffer, bufferSize);
        return NIL_VAL;
    }

    ObjString *rString = copyString(vm, buffer, readSize);
    FREE_ARRAY(vm, char, buffer, bufferSize);

    return OBJ_VAL(rString);
}

static Value closeSocket(DictuVM *vm, int argCount, Value *args) {
    UNUSED(vm);
    if (argCount != 0) {
        runtimeError(vm, "close() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    ObjSocket *sock = AS_SOCKET(args[0]);
    close(sock->socket);

    return NIL_VAL;
}

static Value setSocketOpt(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "setsocketopt() takes 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        runtimeError(vm, "setsocketopt() arguments must be numbers");
        return EMPTY_VAL;
    }

    ObjSocket *sock = AS_SOCKET(args[0]);
    int level = AS_NUMBER(args[1]);
    int option = AS_NUMBER(args[2]);

    if (setsockopt(sock->socket, level, option, &(int){1}, sizeof(int)) == -1) {
        Value module = 0;
        tableGet(&vm->modules, copyString(vm, "Socket", 6), &module);
        SET_ERRNO(AS_MODULE(module));
        return NIL_VAL;
    }

    return TRUE_VAL;
}

#ifdef _WIN32
void cleanupSockets(void) {
    // Calls WSACleanup until an error occurs.
    // Avoids issues if WSAStartup is called multiple times.
    while (!WSACleanup());
}
#endif

ObjModule *createSocketModule(DictuVM *vm) {
    #ifdef _WIN32
    #include "windowsapi.h"

    atexit(cleanupSockets);
    WORD versionWanted = MAKEWORD(2, 2);
    WSADATA wsaData;
    WSAStartup(versionWanted, &wsaData);
    #endif

    ObjString *name = copyString(vm, "Socket", 6);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Socket methods
     */
    defineNative(vm, &module->values, "strerror", strerrorNative);
    defineNative(vm, &module->values, "create", createSocket);

    /**
     * Define Socket properties
     */
    defineNativeProperty(vm, &module->values, "errno", NUMBER_VAL(0));
    defineNativeProperty(vm, &module->values, "AF_INET", NUMBER_VAL(AF_INET));
    defineNativeProperty(vm, &module->values, "SOCK_STREAM", NUMBER_VAL(SOCK_STREAM));
    defineNativeProperty(vm, &module->values, "SOL_SOCKET", NUMBER_VAL(SOL_SOCKET));
    defineNativeProperty(vm, &module->values, "SO_REUSEADDR", NUMBER_VAL(SO_REUSEADDR));

    /**
     * Setup Socket object methods
     */
    defineNative(vm, &vm->socketMethods, "bind", bindSocket);
    defineNative(vm, &vm->socketMethods, "listen", listenSocket);
    defineNative(vm, &vm->socketMethods, "accept", acceptSocket);
    defineNative(vm, &vm->socketMethods, "write", writeSocket);
    defineNative(vm, &vm->socketMethods, "recv", recvSocket);
    defineNative(vm, &vm->socketMethods, "close", closeSocket);
    defineNative(vm, &vm->socketMethods, "setsockopt", setSocketOpt);

    pop(vm);
    pop(vm);

    return module;
}
