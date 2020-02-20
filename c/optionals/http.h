#ifndef dictu_http_h
#define dictu_http_h

#include <curl/curl.h>

#include "optionals.h"
#include "../vm.h"

typedef struct response {
    char *res;
    size_t len;
} Response;

void createHTTPClass();

#endif //dictu_http_h