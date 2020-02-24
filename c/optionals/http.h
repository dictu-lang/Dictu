#ifndef dictu_http_h
#define dictu_http_h

#include <curl/curl.h>

#include "optionals.h"
#include "../vm.h"

typedef struct response {
    char *res;
    ObjList *headers;
    size_t len;
    size_t headerLen;
    long statusCode;
} Response;

void createHTTPClass();

#endif //dictu_http_h