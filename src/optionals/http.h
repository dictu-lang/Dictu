#ifndef dictu_http_h
#define dictu_http_h

#ifndef DISABLE_HTTP
#include <curl/curl.h>
#endif

#include "optionals.h"
#include "../vm/vm.h"

typedef struct response {
    VM *vm;
    ObjList *headers;
    char *res;
    size_t len;
    long statusCode;
} Response;

ObjModule *createHTTPModule(VM *vm);

#endif //dictu_http_h