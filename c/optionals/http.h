#ifndef dictu_http_h
#define dictu_http_h

#ifndef DISABLE_HTTP
#include <curl/curl.h>
#endif

#include "optionals.h"
#include "../vm.h"

typedef struct response {
    VM *vm;
    char *res;
    ObjList *headers;
    size_t len;
    size_t headerLen;
    long statusCode;
} Response;

void createHTTPClass();

#endif //dictu_http_h