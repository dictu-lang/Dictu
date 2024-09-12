#ifndef dictu_http_h
#define dictu_http_h

#include "../optionals.h"
#include "../../vm/vm.h"

#ifndef DISABLE_HTTP
#include <curl/curl.h>
#endif

typedef struct response {
    DictuVM *vm;
    ObjList *headers;
    char *res;
    size_t len;
    long statusCode;
    bool firstIteration;
} Response;

Value createHTTPModule(DictuVM *vm);

#endif //dictu_http_h
