#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "http-source.h"

#define HTTP_METHOD_GET     "GET"
#define HTTP_METHOD_POST    "POST"
#define HTTP_METHOD_PUT     "PUT"
#define HTTP_METHOD_DELETE  "DELETE"
#define HTTP_METHOD_HEAD    "HEAD"
#define HTTP_METHOD_CONNECT "CONNECT"
#define HTTP_METHOD_OPTIONS "OPTIONS"
#define HTTP_METHOD_TRACE   "TRACE"
#define HTTP_METHOD_PATCH   "PATCH"

#define HTTP_STATUS_CODE_CONTINUE            100 // RFC 7231, 6.2.1
#define HTTP_STATUS_CODE_SWITCHING_PROTOCOLS 101 // RFC 7231, 6.2.2
#define HTTP_STATUS_CODE_PROCESSING          102 // RFC 2518, 10.1
#define HTTP_STATUS_CODE_EARLY_HINTS         103 // RFC 8297

#define HTTP_STATUS_CODE_OK                    200 // RFC 7231, 6.3.1
#define HTTP_STATUS_CODE_CREATED               201 // RFC 7231, 6.3.2
#define HTTP_STATUS_CODE_ACCEPTED              202 // RFC 7231, 6.3.3
#define HTTP_STATUS_CODE_NONAUTHORITATIVE_INFO 203 // RFC 7231, 6.3.4
#define HTTP_STATUS_CODE_NO_CONTENT            204 // RFC 7231, 6.3.5
#define HTTP_STATUS_CODE_RESET_CONTENT         205 // RFC 7231, 6.3.6
#define HTTP_STATUS_CODE_PARTIAL_CONTENT       206 // RFC 7233, 4.1
#define HTTP_STATUS_CODE_MULTI_STATUS          207 // RFC 4918, 11.1
#define HTTP_STATUS_CODE_ALREADY_REPORTED      208 // RFC 5842, 7.1
#define HTTP_STATUS_CODE_IM_USED               226 // RFC 3229, 10.4.1

#define HTTP_STATUS_CODE_MULTIPLE_CHOICES   300 // RFC 7231, 6.4.1
#define HTTP_STATUS_CODE_MOVED_PERMANENTLY  301 // RFC 7231, 6.4.2
#define HTTP_STATUS_CODE_FOUND              302 // RFC 7231, 6.4.3
#define HTTP_STATUS_CODE_SEE_OTHER          303 // RFC 7231, 6.4.4
#define HTTP_STATUS_CODE_NOT_MODIFIED       304 // RFC 7232, 4.1
#define HTTP_STATUS_CODE_USE_PROXY          305 // RFC 7231, 6.4.5
#define HTTP_STATUS_CODE_TEMPORARY_REDIRECT 307 // RFC 7231, 6.4.7
#define HTTP_STATUS_CODE_PERMANENT_REDIRECT 308 // RFC 7538, 3

#define HTTP_STATUS_CODE_BAD_REQUEST                     400 // RFC 7231, 6.5.1
#define HTTP_STATUS_CODE_UNAUTHORIZED                    401 // RFC 7235, 3.1
#define HTTP_STATUS_CODE_PAYMENT_REQUIRED                402 // RFC 7231, 6.5.2
#define HTTP_STATUS_CODE_FORBIDDEN                       403 // RFC 7231, 6.5.3
#define HTTP_STATUS_CODE_NOT_FOUND                       404 // RFC 7231, 6.5.4
#define HTTP_STATUS_CODE_METHOD_NOT_ALLOWED              405 // RFC 7231, 6.5.5
#define HTTP_STATUS_CODE_NOT_ACCEPTABLE                  406 // RFC 7231, 6.5.6
#define HTTP_STATUS_CODE_PROXY_AUTH_REQUIRED             407 // RFC 7235, 3.2
#define HTTP_STATUS_CODE_REQUEST_TIMEOUT                 408 // RFC 7231, 6.5.7
#define HTTP_STATUS_CODE_CONFLICT                        409 // RFC 7231, 6.5.8
#define HTTP_STATUS_CODE_GONE                            410 // RFC 7231, 6.5.9
#define HTTP_STATUS_CODE_LENGTH_REQUIRED                 411 // RFC 7231, 6.5.10
#define HTTP_STATUS_CODE_PRECONDITION_FAILED             412 // RFC 7232, 4.2
#define HTTP_STATUS_CODE_REQUEST_ENTITY_TOO_LARGE        413 // RFC 7231, 6.5.11
#define HTTP_STATUS_CODE_REQUEST_URI_TOO_LONG            414 // RFC 7231, 6.5.12
#define HTTP_STATUS_CODE_UNSUPPORTED_MEDIA_TYPE          415 // RFC 7231, 6.5.13
#define HTTP_STATUS_CODE_REQUESTED_RANGE_NOT_SATISFIABLE 416 // RFC 7233, 4.4
#define HTTP_STATUS_CODE_EXPECTATION_FAILED              417 // RFC 7231, 6.5.14
#define HTTP_STATUS_CODE_TEAPOT                          418 // RFC 7168, 2.3.3
#define HTTP_STATUS_CODE_MISDIRECTED_REQUEST             421 // RFC 7540, 9.1.2
#define HTTP_STATUS_CODE_UNPROCESSABLE_ENTITY            422 // RFC 4918, 11.2
#define HTTP_STATUS_CODE_LOCKED                          423 // RFC 4918, 11.3
#define HTTP_STATUS_CODE_FAILED_DEPENDENCY               424 // RFC 4918, 11.4
#define HTTP_STATUS_CODE_TOO_EARLY                       425 // RFC 8470, 5.2.
#define HTTP_STATUS_CODE_UPGRADE_REQUIRED                426 // RFC 7231, 6.5.15
#define HTTP_STATUS_CODE_PRECONDITION_REQUIRED           428 // RFC 6585, 3
#define HTTP_STATUS_CODE_TOO_MANY_REQUESTS               429 // RFC 6585, 4
#define HTTP_STATUS_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE 431 // RFC 6585, 5
#define HTTP_STATUS_CODE_UNAVAILABLE_FOR_LEGAL_REASONS   451 // RFC 7725, 3

#define HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR           500 // RFC 7231, 6.6.1
#define HTTP_STATUS_CODE_NOT_IMPLEMENTED                 501 // RFC 7231, 6.6.2
#define HTTP_STATUS_CODE_BAD_GATEWAY                     502 // RFC 7231, 6.6.3
#define HTTP_STATUS_CODE_SERVICE_UNAVAILABLE             503 // RFC 7231, 6.6.4
#define HTTP_STATUS_CODE_GATEWAY_TIMEOUT                 504 // RFC 7231, 6.6.5
#define HTTP_STATUS_CODE_HTTP_VERSION_NOT_SUPPORTED      505 // RFC 7231, 6.6.6
#define HTTP_STATUS_CODE_VARIANT_ALSO_NEGOTIATES         506 // RFC 2295, 8.1
#define HTTP_STATUS_CODE_INSUFFICIENT_STORAGE            507 // RFC 4918, 11.5
#define HTTP_STATUS_CODE_LOOP_DETECTED                   508 // RFC 5842, 7.2
#define HTTP_STATUS_CODE_NOT_EXTENDED                    510 // RFC 2774, 7
#define HTTP_STATUS_CODE_NETWORK_AUTHENTICATION_REQUIRED 511 // RFC 6585, 6

#define HTTP_STATUS_MESSAGE_CONTINUE                        "Continue"
#define HTTP_STATUS_MESSAGE_SWITCHING_PROTOCOLS             "Switching Protocols"
#define HTTP_STATUS_MESSAGE_PROCESS                         "Processing"
#define HTTP_STATUS_MESSAGE_EARLY_HINTS                     "Early Hints"
#define HTTP_STATUS_MESSAGE_OK                              "OK"
#define HTTP_STATUS_MESSAGE_CREATED                         "Created"
#define HTTP_STATUS_MESSAGE_ACCEPTED                        "Accepted"
#define HTTP_STATUS_MESSAGE_NONAUTHORITATIVE_INFO           "Non-Authoritative Information"
#define HTTP_STATUS_MESSAGE_NO_CONTENT                      "No Content"
#define HTTP_STATUS_MESSAGE_RESET_CONTENT                   "Reset Content"
#define HTTP_STATUS_MESSAGE_PARTIAL_CONTENT                 "Partial Content"
#define HTTP_STATUS_MESSAGE_MULTI_STATUS                    "Multi-Status"
#define HTTP_STATUS_MESSAGE_ALREADY_REPORTED                "Already Reported"
#define HTTP_STATUS_MESSAGE_IM_USED                         "IM Used"
#define HTTP_STATUS_MESSAGE_MULTIPLE_CHOICES                "Multiple Choices"
#define HTTP_STATUS_MESSAGE_MOVED_PERMANENTLY               "Moved Permanently"
#define HTTP_STATUS_MESSAGE_FOUND                           "Found"
#define HTTP_STATUS_MESSAGE_SEE_OTHER                       "See Other"
#define HTTP_STATUS_MESSAGE_NOT_MODIFIED                    "Not Modified"
#define HTTP_STATUS_MESSAGE_USE_PROXY                       "Use Proxy"
#define HTTP_STATUS_MESSAGE_TEMPORARY_REDIRECT              "Temporary Redirect"
#define HTTP_STATUS_MESSAGE_PERMANENT_REDIRECT              "Permanent Redirect"
#define HTTP_STATUS_MESSAGE_BAD_REQUEST                     "Bad Request"
#define HTTP_STATUS_MESSAGE_UNAUTHORIZED                    "Unauthorized"
#define HTTP_STATUS_MESSAGE_PAYMENT_REQUIRED                "Payment Required"
#define HTTP_STATUS_MESSAGE_FORBIDDEN                       "Forbidden"
#define HTTP_STATUS_MESSAGE_NOT_FOUND                       "Not Found"
#define HTTP_STATUS_MESSAGE_METHOD_NOT_ALLOWED              "Method Not Allowed"
#define HTTP_STATUS_MESSAGE_NOT_ACCEPTABLE                  "Not Acceptable"
#define HTTP_STATUS_MESSAGE_PROXY_AUTH_REQUIRED             "Proxy Authentication Required"
#define HTTP_STATUS_MESSAGE_REQUEST_TIMEOUT                 "Request Timeout"
#define HTTP_STATUS_MESSAGE_CONFLICT                        "Conflict"
#define HTTP_STATUS_MESSAGE_GONE                            "Gone"
#define HTTP_STATUS_MESSAGE_LENGTH_REQUIRED                 "Length Required"
#define HTTP_STATUS_MESSAGE_PRECONDITION_FAILED             "Precondition Failed"
#define HTTP_STATUS_MESSAGE_REQUEST_ENTITY_TOO_LARGE        "Request Entity Too Large"
#define HTTP_STATUS_MESSAGE_REQUEST_URI_TOO_LONG            "Request URI Too Long"
#define HTTP_STATUS_MESSAGE_UNSUPPORTED_MEDIA_TYPE          "Unsupported Media Type"
#define HTTP_STATUS_MESSAGE_REQUESTED_RANGE_NOT_SATISFIABLE "Requested Range Not Satisfiable"
#define HTTP_STATUS_MESSAGE_EXPECTATION_FAILED              "Expectation Failed"
#define HTTP_STATUS_MESSAGE_TEAPOT                          "I'm a teapot"
#define HTTP_STATUS_MESSAGE_MISDIRECTED_REQUEST             "Misdirected Request"
#define HTTP_STATUS_MESSAGE_UNPROCESSABLE_ENTITY            "Unprocessable Entity"
#define HTTP_STATUS_MESSAGE_LOCKED                          "Locked"
#define HTTP_STATUS_MESSAGE_FAILED_DEPENDENCY               "Failed Dependency"
#define HTTP_STATUS_MESSAGE_TOO_EARLY                       "Too Early"
#define HTTP_STATUS_MESSAGE_UPGRADE_REQIUIRED               "Upgrade Required"
#define HTTP_STATUS_MESSAGE_PRECONDITION_REQUIRED           "Precondition Required"
#define HTTP_STATUS_MESSAGE_TOO_MANY_REQUESTS               "Too Many Requests"
#define HTTP_STATUS_MESSAGE_REQUEST_HEADER_FIELDS_TOO_LARGE "Request Header Fields Too Large"
#define HTTP_STATUS_MESSAGE_UNAVAILABLE_FOR_LEGAL_REASONS   "Unavailable For Legal Reasons"
#define HTTP_STATUS_MESSAGE_INTERNAL_SERVER_ERROR           "Internal Server Error"
#define HTTP_STATUS_MESSAGE_NOT_IMPLEMENTED                 "Not Implemented"
#define HTTP_STATUS_MESSAGE_BAD_GATEWAY                     "Bad Gateway"
#define HTTP_STATUS_MESSAGE_UNAVAILABLE                     "Service Unavailable"
#define HTTP_STATUS_MESSAGE_GATEWAY_TIMEOUT                 "Gateway Timeout"
#define HTTP_STATUS_MESSAGE_HTTP_VERSION_NOT_SUPPORTED      "HTTP Version Not Supported"
#define HTTP_STATUS_MESSAGE_VARIAN_ALSO_NEGOTIATES          "Variant Also Negotiates"
#define HTTP_STATUS_MESSAGE_INSUFFICIENT_STORAGE            "Insufficient Storage"
#define HTTP_STATUS_MESSAGE_LOOP_DETECTED                   "Loop Detected"
#define HTTP_STATUS_MESSAGE_NOT_EXTENDED                    "Not Extended"
#define HTTP_STATUS_MESSAGE_NETWORK_AUTHENTICATION_REQUIRED "Network Authentication Required"

#define HTTP_REQUEST_HEADER_AIM                            "A-IM"
#define HTTP_REQUEST_HEADER_ACCEPT                         "Accept"
#define HTTP_REQUEST_HEADER_ACCEPT_CHARSET                 "Accept-Charset"
#define HTTP_REQUEST_HEADER_ACCEPT_DATETIME                "Accept-Datetime"
#define HTTP_REQUEST_HEADER_ACCEPT_ENCODING                "Accept-Encoding"
#define HTTP_REQUEST_HEADER_ACCEPT_LANGUAGE                "Accept-Language"
#define HTTP_REQUEST_HEADER_ACCEPT_CONTROL_REQUEST_METHOD  "Access-Control-Request-Method"
#define HTTP_REQUEST_HEADER_ACCEPT_CONTROL_REQUEST_HEADERS "Access-Control-Request-Headers"
#define HTTP_REQUEST_HEADER_AUTHORIZATION                  "Authorization"
#define HTTP_REQUEST_HEADER_CACHE_CONTROL                  "Cache-Control"
#define HTTP_REQUEST_HEADER_CONNECTION                     "Connection"
#define HTTP_REQUEST_HEADER_CONTENT_ENCODING               "Content-Encoding"
#define HTTP_REQUEST_HEADER_CONTENT_LENGTH                 "Content-Length"
#define HTTP_REQUEST_HEADER_CONTENT_MD5                    "Content-MD5"
#define HTTP_REQUEST_HEADER_CONTENT_TYPE                   "Content-Type"
#define HTTP_REQUEST_HEADER_COOKIE                         "Cookie"
#define HTTP_REQUEST_HEADER_DATE                           "Date"
#define HTTP_REQUEST_HEADER_EXPECT                         "Expect"
#define HTTP_REQUEST_HEADER_FORWARDED                      "Forwarded"
#define HTTP_REQUEST_HEADER_FROM                           "From"
#define HTTP_REQUEST_HEADER_HOST                           "Host"	
#define HTTP_REQUEST_HEADER_HTTP2_SETTINGS                 "HTTP2-Settings"
#define HTTP_REQUEST_HEADER_IF_MATCH                       "If-Match"
#define HTTP_REQUEST_HEADER_IF_MODIFIED_SINCE              "If-Modified-Since"
#define HTTP_REQUEST_HEADER_IF_NONE_MATCH                  "If-None-Match"
#define HTTP_REQUEST_HEADER_IF_RANGE                       "If-Range"
#define HTTP_REQUEST_HEADER_IF_UNMODIFIED_SINCE            "If-Unmodified-Since"
#define HTTP_REQUEST_HEADER_MAX_FORWARDS                   "Max-Forwards"
#define HTTP_REQUEST_HEADER_PRAGMA                         "Pragma"
#define HTTP_REQUEST_HEADER_PROXY_AUTHORIZATION            "Proxy-Authorization"
#define HTTP_REQUEST_HEADER_RANGE                          "Range"
#define HTTP_REQUEST_HEADER_REFERRER                       "Referer"
#define HTTP_REQUEST_HEADER_TE                             "TE"
#define HTTP_REQUEST_HEADER_TRAILER                        "Trailer"	
#define HTTP_REQUEST_HEADER_TRANSFER_ENCODING              "Transfer-Encoding"
#define HTTP_REQUEST_HEADER_USER_AGENT                     "User-Agent"
#define HTTP_REQUEST_HEADER_UPGRADE                        "Upgrade"
#define HTTP_REQUEST_HEADER_WARNING                        "Warning"

#define HTTP_RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN      "Access-Control-Allow-Origin"
#define HTTP_RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_CREDENTIALS HTTP_RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN
#define HTTP_RESPONSE_HEADER_ACCESS_CONTROL_EXPOSE_HEADERS    HTTP_RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN
#define HTTP_RESPONSE_HEADER_ACCESS_CONTROL_MAX_AGE           HTTP_RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN
#define HTTP_RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_METHODS     HTTP_RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN
#define HTTP_RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_HEADERS     HTTP_RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN
#define HTTP_RESPONSE_HEADER_ACCEPT_PATCH                     "Accept-Patch"
#define HTTP_RESPONSE_HEADER_ACCEPT_RANGES                    "Accept-Ranges"
#define HTTP_RESPONSE_HEADER_AGE                              "Age"
#define HTTP_RESPONSE_HEADER_ALLOW                            "Allow"	
#define HTTP_RESPONSE_HEADER_ALT_SVC                          "Alt-Svc"
#define HTTP_RESPONSE_HEADER_CACHE_CONTROL                    "Cache-Control"
#define HTTP_RESPONSE_HEADER_CONNECTION                       "Connection"
#define HTTP_RESPONSE_HEADER_CONTENT_DISPOSITION              "Content-Disposition"
#define HTTP_RESPONSE_HEADER_CONTENT_ENCODING                 "Content-Encoding"
#define HTTP_RESPONSE_HEADER_CONTENT_LANGUAGE                 "Content-Language"
#define HTTP_RESPONSE_HEADER_CONTENT_LENGTH                   "Content-Length"
#define HTTP_RESPONSE_HEADER_CONTENT_LOCATION                 "Content-Location"
#define HTTP_RESPONSE_HEADER_CONTENT_MD5                      "Content-MD5"
#define HTTP_RESPONSE_HEADER_CONTENT_RANGE                    "Content-Range"
#define HTTP_RESPONSE_HEADER_CONTENT_TYPE                     "Content-Type"
#define HTTP_RESPONSE_HEADER_DATE                             "Date"
#define HTTP_RESPONSE_HEADER_DELTA_BASE                       "Delta-Base"
#define HTTP_RESPONSE_HEADER_ETAG                             "ETag"
#define HTTP_RESPONSE_HEADER_EXPIRES                          "Expires"
#define HTTP_RESPONSE_HEADER_IM                               "IM"
#define HTTP_RESPONSE_HEADER_LAST_MODIFIED                    "Last-Modified"
#define HTTP_RESPONSE_HEADER_LINK                             "Link"
#define HTTP_RESPONSE_HEADER_LOCATION                         "Location"
#define HTTP_RESPONSE_HEADER_P3P                              "P3P"
#define HTTP_RESPONSE_HEADER_PRAGMA                           "Pragma"
#define HTTP_RESPONSE_HEADER_PROXY_AUTHENTICATE               "Proxy-Authenticate"
#define HTTP_RESPONSE_HEADER_PUBLIC_KEY_PINS                  "Public-Key-Pins"
#define HTTP_RESPONSE_HEADER_RETRY_AFTER                      "Retry-After"
#define HTTP_RESPONSE_HEADER_SET_COOKIE                       "Set-Cookie"
#define HTTP_RESPONSE_HEADER_STRICT_TRANSPORT_SECURITY        "Strict-Transport-Security"
#define HTTP_RESPONSE_HEADER_TRAILER                          "Trailer"
#define HTTP_RESPONSE_HEADER_TRANSFER_ENCODING                "Transfer-Encoding"
#define HTTP_RESPONSE_HEADER_TK                               "Tk"
#define HTTP_RESPONSE_HEADER_UPGRADE                          "Upgrade"
#define HTTP_RESPONSE_HEADER_VARY                             "Vary"
#define HTTP_RESPONSE_HEADER_VIA                              "Via"
#define HTTP_RESPONSE_HEADER_WARNING                          "Warning"
#define HTTP_RESPONSE_HEADER_WWW_AUTHENTICATE                 "WWW-Authenticate"
#define HTTP_RESPONSE_HEADER_X_FRAME_OPTIONS                  "X-Frame-Options"

#define HTTP_MAX_HEADER_BYTES 1 << 20 // 1 MB

#define DEFAULT_REQUEST_TIMEOUT 20

static void createResponse(DictuVM *vm, Response *response) {
    response->vm = vm;
    response->headers = newList(vm);
    // Push to stack to avoid GC
    push(vm, OBJ_VAL(response->headers));

    response->len = 0;
    response->res = NULL;
}

static size_t writeResponse(char *ptr, size_t size, size_t nmemb, void *data) {
    Response *response = (Response *) data;
    size_t new_len = response->len + size * nmemb;
    response->res = GROW_ARRAY(response->vm, response->res, char, response->len, new_len + 1);
    if (response->res == NULL) {
        printf("Unable to allocate memory\n");
        exit(71);
    }
    memcpy(response->res + response->len, ptr, size * nmemb);
    response->res[new_len] = '\0';
    response->len = new_len;

    return size * nmemb;
}

static size_t writeHeaders(char *ptr, size_t size, size_t nitems, void *data) {
    Response *response = (Response *) data;
    // if nitems equals 2 its an empty header
    if (nitems != 2) {
        Value header = OBJ_VAL(copyString(response->vm, ptr, (nitems - 2) * size));
        // Push to stack to avoid GC
        push(response->vm, header);
        writeValueArray(response->vm, &response->headers->values, header);
        pop(response->vm);
    }
    return size * nitems;
}

static char *dictToPostArgs(ObjDict *dict) {
    int len = 100;
    char *ret = malloc(sizeof(char) * len);
    int currentLen = 0;

    for (int i = 0; i <= dict->capacityMask; i++) {
        DictItem *entry = &dict->entries[i];
        if (IS_EMPTY(entry->key)) {
            continue;
        }

        char *key;
        if (IS_STRING(entry->key)) {
            key = AS_CSTRING(entry->key);
        } else {
            key = valueToString(entry->key);
        }

        char *value;
        if (IS_STRING(entry->value)) {
            value = AS_CSTRING(entry->value);
        } else {
            value = valueToString(entry->value);
        }

        int keyLen = strlen(key);
        int valLen = strlen(value);

        if (currentLen + keyLen + valLen > len) {
            len = len * 2 + keyLen + valLen;
            ret = realloc(ret, len);

            if (ret == NULL) {
                printf("Unable to allocate memory\n");
                exit(71);
            }
        }

        memcpy(ret + currentLen, key, keyLen);
        currentLen += keyLen;
        memcpy(ret + currentLen, "=", 1);
        currentLen += 1;
        memcpy(ret + currentLen, value, valLen);
        currentLen += valLen;
        memcpy(ret + currentLen, "&", 1);
        currentLen += 1;

        if (!IS_STRING(entry->key)) {
            free(key);
        }
        if (!IS_STRING(entry->value)) {
            free(value);
        }
    }

    ret[currentLen] = '\0';

    return ret;
}

static bool setRequestHeaders(DictuVM *vm, struct curl_slist *list, CURL *curl, ObjList *headers) {
    if (headers->values.count == 0) {
        return true;
    }

    for (int i = 0; i < headers->values.count; ++i) {
        if (!IS_STRING(headers->values.values[i])) {
            runtimeError(vm, "Headers list must only contain strings");
            return false;
        }

        list = curl_slist_append(list, AS_CSTRING(headers->values.values[i]));
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

    return true;
}

static ObjInstance *endRequest(DictuVM *vm, CURL *curl, Response response, bool cleanup) {
    // Get status code
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.statusCode);
    ObjString *content;
    if (response.res != NULL) {
        content = takeString(vm, response.res, response.len);
    } else {
        content = copyString(vm, "", 0);
    }

    // Push to stack to avoid GC
    push(vm, OBJ_VAL(content));

    Value rawModule;
    tableGet(&vm->modules, copyString(vm, "HTTP", 4), &rawModule);

    Value rawResponseClass;
    tableGet(&AS_MODULE(rawModule)->values, copyString(vm, "Response", 8), &rawResponseClass);

    ObjInstance *responseInstance = newInstance(vm, AS_CLASS(rawResponseClass));
    // Push to stack to avoid GC
    push(vm, OBJ_VAL(responseInstance));

    ObjString *string = copyString(vm, "content", 7);
    push(vm, OBJ_VAL(string));
    tableSet(vm, &responseInstance->publicAttributes, string, OBJ_VAL(content));
    pop(vm);

    string = copyString(vm, "headers", 7);
    push(vm, OBJ_VAL(string));
    tableSet(vm, &responseInstance->publicAttributes, string, OBJ_VAL(response.headers));
    pop(vm);

    string = copyString(vm, "statusCode", 10);
    push(vm, OBJ_VAL(string));
    tableSet(vm, &responseInstance->publicAttributes, string, NUMBER_VAL(response.statusCode));
    pop(vm);

    // Pop instance
    pop(vm);
    // Pop content
    pop(vm);
    // Pop headers from createResponse
    pop(vm);

    if (cleanup) {
        /* always cleanup */
        curl_easy_cleanup(curl);
        curl_global_cleanup();
    }
    
    return responseInstance;
}

static Value get(DictuVM *vm, int argCount, Value *args) {
    if (argCount < 0 || argCount > 3) {
        runtimeError(vm, "get() takes between 1 and 3 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    long timeout = DEFAULT_REQUEST_TIMEOUT;
    ObjList *headers = NULL;

    if (argCount == 3) {
        if (!IS_NUMBER(args[2])) {
            runtimeError(vm, "Timeout passed to get() must be a number.");
            return EMPTY_VAL;
        }

        timeout = AS_NUMBER(args[2]);
        argCount--;
    }

    if (argCount == 2) {
        if (!IS_LIST(args[1])) {
            runtimeError(vm, "Headers passed to get() must be a list.");
            return EMPTY_VAL;
        }

        headers = AS_LIST(args[1]);
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "URL passed to get() must be a string.");
        return EMPTY_VAL;
    }

    CURL *curl;
    CURLcode curlResponse;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        Response response;
        createResponse(vm, &response);
        char *url = AS_CSTRING(args[0]);

        struct curl_slist *list = NULL;

        if (headers) {
            if (!setRequestHeaders(vm, list, curl, headers)) {
                curl_slist_free_all(list);
                return EMPTY_VAL;
            }
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeResponse);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writeHeaders);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        /* Perform the request, res will get the return code */
        curlResponse = curl_easy_perform(curl);

        if (headers) {
            curl_slist_free_all(list);
        }

        /* Check for errors */
        if (curlResponse != CURLE_OK) {
            /* always cleanup */
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            pop(vm);

            char *errorString = (char *) curl_easy_strerror(curlResponse);
            return newResultError(vm, errorString);
        }

        return newResultSuccess(vm, OBJ_VAL(endRequest(vm, curl, response, true)));
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    pop(vm);

    char *errorString = (char *) curl_easy_strerror(CURLE_FAILED_INIT);
    return newResultError(vm, errorString);
}

static Value post(DictuVM *vm, int argCount, Value *args) {
    if (argCount < 1 || argCount > 4) {
        runtimeError(vm, "post() takes between 1 and 4 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    long timeout = DEFAULT_REQUEST_TIMEOUT;
    ObjDict *postValuesDict = NULL;
    ObjString *postValueString = NULL;
    ObjList *headers = NULL;

    if (argCount == 4) {
        if (!IS_NUMBER(args[3])) {
            runtimeError(vm, "Timeout passed to post() must be a number.");
            return EMPTY_VAL;
        }

        timeout = (long) AS_NUMBER(args[3]);
        argCount--;
    }

    if (argCount == 3) {
        if (!IS_LIST(args[2])) {
            runtimeError(vm, "Headers passed to post() must be a list.");
            return EMPTY_VAL;
        }

        headers = AS_LIST(args[2]);
        argCount--;
    }

    if (argCount == 2) {
        if (IS_DICT(args[1])) {
            postValuesDict = AS_DICT(args[1]);
        } else if (IS_STRING(args[1])) {
            postValueString = AS_STRING(args[1]);
        } else {
            runtimeError(vm, "Post values passed to post() must be a dictionary or a string.");
            return EMPTY_VAL;
        }
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "URL passed to post() must be a string.");
        return EMPTY_VAL;
    }

    CURL *curl;
    CURLcode curlResponse;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        Response response;
        createResponse(vm, &response);
        char *url = AS_CSTRING(args[0]);
        char *postValue = "";

        struct curl_slist *list = NULL;

        if (headers) {
            if (!setRequestHeaders(vm, list, curl, headers)) {
                curl_slist_free_all(list);
                return EMPTY_VAL;
            }
        }

        if (postValuesDict != NULL) {
            postValue = dictToPostArgs(postValuesDict);
        } else if (postValueString != NULL) {
            postValue = postValueString->chars;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postValue);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeResponse);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writeHeaders);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        /* Perform the request, res will get the return code */
        curlResponse = curl_easy_perform(curl);

        if (headers) {
            curl_slist_free_all(list);
        }

        if (postValuesDict != NULL) {
            free(postValue);
        }

        if (curlResponse != CURLE_OK) {
            /* always cleanup */
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            pop(vm);

            char *errorString = (char *) curl_easy_strerror(curlResponse);
            return newResultError(vm, errorString);
        }

        return newResultSuccess(vm, OBJ_VAL(endRequest(vm, curl, response, true)));
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    pop(vm);

    char *errorString = (char *) curl_easy_strerror(CURLE_FAILED_INIT);
    return newResultError(vm, errorString);
}

static Value put(DictuVM *vm, int argCount, Value *args) {
    if (argCount < 1 || argCount > 4) {
        runtimeError(vm, "put() takes between 1 and 4 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    long timeout = DEFAULT_REQUEST_TIMEOUT;
    ObjDict *putValuesDict = NULL;
    ObjString *putValueString = NULL;
    ObjList *headers = NULL;

    if (argCount == 4) {
        if (!IS_NUMBER(args[3])) {
            runtimeError(vm, "Timeout passed to put() must be a number.");
            return EMPTY_VAL;
        }

        timeout = (long) AS_NUMBER(args[3]);
        argCount--;
    }

    if (argCount == 3) {
        if (!IS_LIST(args[2])) {
            runtimeError(vm, "Headers passed to put() must be a list.");
            return EMPTY_VAL;
        }

        headers = AS_LIST(args[2]);
        argCount--;
    }

    if (argCount == 2) {
        if (IS_DICT(args[1])) {
            putValuesDict = AS_DICT(args[1]);
        } else if (IS_STRING(args[1])) {
            putValueString = AS_STRING(args[1]);
        } else {
            runtimeError(vm, "Put values passed to put() must be a dictionary or a string.");
            return EMPTY_VAL;
        }
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "URL passed to put() must be a string.");
        return EMPTY_VAL;
    }

    CURL *curl;
    CURLcode curlResponse;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        Response response;
        createResponse(vm, &response);
        char *url = AS_CSTRING(args[0]);
        char *putValue = "";

        struct curl_slist *list = NULL;

        if (headers) {
            if (!setRequestHeaders(vm, list, curl, headers)) {
                curl_slist_free_all(list);
                return EMPTY_VAL;
            }
        }

        if (putValuesDict != NULL) {
            putValue = dictToPostArgs(putValuesDict);
        } else if (putValueString != NULL) {
            putValue = putValueString->chars;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, putValue);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeResponse);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writeHeaders);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        /* Perform the request, res will get the return code */
        curlResponse = curl_easy_perform(curl);

        if (headers) {
            curl_slist_free_all(list);
        }

        if (putValuesDict != NULL) {
            free(putValue);
        }

        if (curlResponse != CURLE_OK) {
            /* always cleanup */
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            pop(vm);

            char *errorString = (char *) curl_easy_strerror(curlResponse);
            return newResultError(vm, errorString);
        }

        return newResultSuccess(vm, OBJ_VAL(endRequest(vm, curl, response, true)));
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    pop(vm);

    char *errorString = (char *) curl_easy_strerror(CURLE_FAILED_INIT);
    return newResultError(vm, errorString);
}

static Value head(DictuVM *vm, int argCount, Value *args) {
    if (argCount < 0 || argCount > 3) {
        runtimeError(vm, "head() takes between 1 and 3 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    long timeout = DEFAULT_REQUEST_TIMEOUT;
    ObjList *headers = NULL;

    if (argCount == 3) {
        if (!IS_NUMBER(args[2])) {
            runtimeError(vm, "Timeout passed to head() must be a number.");
            return EMPTY_VAL;
        }

        timeout = AS_NUMBER(args[2]);
        argCount--;
    }

    if (argCount == 2) {
        if (!IS_LIST(args[1])) {
            runtimeError(vm, "Headers passed to head() must be a list.");
            return EMPTY_VAL;
        }

        headers = AS_LIST(args[1]);
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "URL passed to head() must be a string.");
        return EMPTY_VAL;
    }

    CURL *curl;
    CURLcode curlResponse;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        Response response;
        createResponse(vm, &response);
        char *url = AS_CSTRING(args[0]);

        struct curl_slist *list = NULL;

        if (headers) {
            if (!setRequestHeaders(vm, list, curl, headers)) {
                curl_slist_free_all(list);
                return EMPTY_VAL;
            }
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writeHeaders);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        /* Perform the request, res will get the return code */
        curlResponse = curl_easy_perform(curl);

        if (headers) {
            curl_slist_free_all(list);
        }

        /* Check for errors */
        if (curlResponse != CURLE_OK) {
            /* always cleanup */
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            pop(vm);

            char *errorString = (char *) curl_easy_strerror(curlResponse);
            return newResultError(vm, errorString);
        }

        return newResultSuccess(vm, OBJ_VAL(endRequest(vm, curl, response, true)));
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    pop(vm);

    char *errorString = (char *) curl_easy_strerror(CURLE_FAILED_INIT);
    return newResultError(vm, errorString);
}

typedef struct {
    CURL *curl;
} HttpClient;

#define AS_HTTP_CLIENT(v) ((HttpClient*)AS_ABSTRACT(v)->data)

struct curl_slist *headerChunk = NULL;

void freeHttpClient(DictuVM *vm, ObjAbstract *abstract) {
    HttpClient *httpClient = (HttpClient*)abstract->data;
    
    curl_easy_cleanup(httpClient->curl);
    curl_global_cleanup();

    FREE(vm, HttpClient, abstract->data);
}

char *httpClientToString(ObjAbstract *abstract) {
    UNUSED(abstract);

    char *httpClientString = malloc(sizeof(char) * 13);
    snprintf(httpClientString, 13, "<HttpClient>");
    return httpClientString;
}

static Value httpClientSetTimeout(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "setTimeout() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm, "timeout value must be a number");
        return EMPTY_VAL;
    }

    HttpClient *httpClient = AS_HTTP_CLIENT(args[0]);

    curl_easy_setopt(httpClient->curl, CURLOPT_TIMEOUT, (long)AS_NUMBER(args[1]));

    return NIL_VAL;
}

static Value httpClientSetInsecure(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "setInsecure() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_BOOL(args[1])) {
        runtimeError(vm, "insecure value must be a bool");
        return EMPTY_VAL;
    }

    HttpClient *httpClient = AS_HTTP_CLIENT(args[0]);

    if (AS_BOOL(args[1])) {
        curl_easy_setopt(httpClient->curl, CURLOPT_SSL_VERIFYSTATUS, 0);
        curl_easy_setopt(httpClient->curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(httpClient->curl, CURLOPT_SSL_VERIFYPEER, 0);
    } else {
        curl_easy_setopt(httpClient->curl, CURLOPT_SSL_VERIFYSTATUS, 1);
        curl_easy_setopt(httpClient->curl, CURLOPT_SSL_VERIFYHOST, 1);
        curl_easy_setopt(httpClient->curl, CURLOPT_SSL_VERIFYPEER, 1);
    }

    return NIL_VAL;
}

static Value httpClientSetFollowRedirects(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "setFollowRedirects() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_BOOL(args[1])) {
        runtimeError(vm, "setFollowRedirects value must be a bool");
        return EMPTY_VAL;
    }

    HttpClient *httpClient = AS_HTTP_CLIENT(args[0]);

    if (AS_BOOL(args[1])) {
        curl_easy_setopt(httpClient->curl, CURLOPT_FOLLOWLOCATION, 1L);
    } else {
        curl_easy_setopt(httpClient->curl, CURLOPT_FOLLOWLOCATION, 0L);
    }

    return NIL_VAL;
}

static Value httpClientSetHeaders(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "setHeaders() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_LIST(args[1])) {
        runtimeError(vm, "headers value must be a ist");
        return EMPTY_VAL;
    }

    if (IS_EMPTY(args[1])) {
        return EMPTY_VAL;
    }

    HttpClient *httpClient = AS_HTTP_CLIENT(args[0]);

    ObjList *headers = AS_LIST(args[1]);

    headerChunk = NULL;

    for (int h = 0; h < headers->values.count; h++) {
        headerChunk = curl_slist_append(headerChunk, AS_STRING(headers->values.values[h])->chars);
    }
    
    curl_easy_setopt(httpClient->curl, CURLOPT_HTTPHEADER, headerChunk);

    return NIL_VAL;
}

static Value httpClientSetKeyFile(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "setKeyFile() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "keyFile value must be a string");
        return EMPTY_VAL;
    }

    HttpClient *httpClient = AS_HTTP_CLIENT(args[0]);

    curl_easy_setopt(httpClient->curl, CURLOPT_SSLKEY, AS_STRING(args[1])->chars);

    return NIL_VAL;
}

static Value httpClientSetCertFile(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "setCertFile() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "certFile value must be a string");
        return EMPTY_VAL;
    }

    HttpClient *httpClient = AS_HTTP_CLIENT(args[0]);

    curl_easy_setopt(httpClient->curl, CURLOPT_SSLKEY, AS_STRING(args[1])->chars);

    return NIL_VAL;
}

static Value httpClientSetKeyPass(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "setKeyPasswd() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "keyPasswd value must be a string");
        return EMPTY_VAL;
    }

    HttpClient *httpClient = AS_HTTP_CLIENT(args[0]);

    curl_easy_setopt(httpClient->curl, CURLOPT_SSLKEY, AS_STRING(args[1])->chars);

    return NIL_VAL;
}

static Value httpClientGet(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "get() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "URL passed to get() must be a string.");
        return EMPTY_VAL;
    }

    HttpClient *httpClient = AS_HTTP_CLIENT(args[0]);

    CURLcode curlResponse;

    if (httpClient) {
        Response response;
        createResponse(vm, &response);
        char *url = AS_CSTRING(args[1]);

        curl_easy_setopt(httpClient->curl, CURLOPT_URL, url);
        curl_easy_setopt(httpClient->curl, CURLOPT_ACCEPT_ENCODING, "gzip");
        curl_easy_setopt(httpClient->curl, CURLOPT_WRITEFUNCTION, writeResponse);
        curl_easy_setopt(httpClient->curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(httpClient->curl, CURLOPT_HEADERDATA, &response);

        curlResponse = curl_easy_perform(httpClient->curl);

        if (curlResponse != CURLE_OK) {
            pop(vm);

            char *errorString = (char *)curl_easy_strerror(curlResponse);
            return newResultError(vm, errorString);
        }

        return newResultSuccess(vm, OBJ_VAL(endRequest(vm, httpClient->curl, response, false)));
    }

    pop(vm);

    char *errorString = (char *)curl_easy_strerror(CURLE_FAILED_INIT);
    return newResultError(vm, errorString);
}

static Value httpClientPost(DictuVM *vm, int argCount, Value *args) {
    if (argCount < 1 || argCount > 2) {
        runtimeError(vm, "post() takes at least 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    ObjDict *postValuesDict = NULL;
    ObjString *postValueString = NULL;

    if (argCount == 2) {
        if (IS_DICT(args[2])) {
            postValuesDict = AS_DICT(args[2]);
        } else if (IS_STRING(args[2])) {
            postValueString = AS_STRING(args[2]);
        } else {
            runtimeError(vm, "Post values passed to post() must be a dictionary or a string.");
            return EMPTY_VAL;
        }
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "URL passed to post() must be a string.");
        return EMPTY_VAL;
    }

    CURLcode curlResponse;
    
    HttpClient *httpClient = AS_HTTP_CLIENT(args[0]);

    if (httpClient) {
        Response response;
        createResponse(vm, &response);
        char *url = AS_CSTRING(args[1]);
        char *postValue = "";

        if (postValuesDict != NULL) {
            postValue = dictToPostArgs(postValuesDict);
        } else if (postValueString != NULL) {
            postValue = postValueString->chars;
        }

        curl_easy_setopt(httpClient->curl, CURLOPT_URL, url);
        curl_easy_setopt(httpClient->curl, CURLOPT_ACCEPT_ENCODING, "gzip");
        curl_easy_setopt(httpClient->curl, CURLOPT_POSTFIELDS, postValue);
        curl_easy_setopt(httpClient->curl, CURLOPT_WRITEFUNCTION, writeResponse);
        curl_easy_setopt(httpClient->curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(httpClient->curl, CURLOPT_HEADERDATA, &response);

        curlResponse = curl_easy_perform(httpClient->curl);

        if (postValuesDict != NULL) {
            free(postValue);
        }

        if (curlResponse != CURLE_OK) {
            pop(vm);

            char *errorString = (char *)curl_easy_strerror(curlResponse);
            return newResultError(vm, errorString);
        }

        return newResultSuccess(vm, OBJ_VAL(endRequest(vm, httpClient->curl, response, false)));
    }

    pop(vm);

    char *errorString = (char *) curl_easy_strerror(CURLE_FAILED_INIT);
    return newResultError(vm, errorString);
}

static Value httpClientPut(DictuVM *vm, int argCount, Value *args) {
    if (argCount < 1 || argCount > 2) {
        runtimeError(vm, "put() takes at least 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    ObjDict *putValuesDict = NULL;
    ObjString *putValueString = NULL;

    if (argCount == 2) {
        if (IS_DICT(args[2])) {
            putValuesDict = AS_DICT(args[2]);
        } else if (IS_STRING(args[2])) {
            putValueString = AS_STRING(args[2]);
        } else {
            runtimeError(vm, "Put values passed to put() must be a dictionary or a string.");
            return EMPTY_VAL;
        }
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "URL passed to put() must be a string.");
        return EMPTY_VAL;
    }

    CURLcode curlResponse;

    HttpClient *httpClient = AS_HTTP_CLIENT(args[0]);

    if (httpClient) {
        Response response;
        createResponse(vm, &response);
        char *url = AS_CSTRING(args[1]);
        char *putValue = "";

        if (putValuesDict != NULL) {
            putValue = dictToPostArgs(putValuesDict);
        } else if (putValueString != NULL) {
            putValue = putValueString->chars;
        }

        curl_easy_setopt(httpClient->curl, CURLOPT_URL, url);
        curl_easy_setopt(httpClient->curl, CURLOPT_ACCEPT_ENCODING, "gzip");
        curl_easy_setopt(httpClient->curl, CURLOPT_POSTFIELDS, putValue);
        curl_easy_setopt(httpClient->curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(httpClient->curl, CURLOPT_WRITEFUNCTION, writeResponse);
        curl_easy_setopt(httpClient->curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(httpClient->curl, CURLOPT_HEADERDATA, &response);

        curlResponse = curl_easy_perform(httpClient->curl);

        if (putValuesDict != NULL) {
            free(putValue);
        }

        if (curlResponse != CURLE_OK) {
            pop(vm);

            char *errorString = (char *)curl_easy_strerror(curlResponse);
            return newResultError(vm, errorString);
        }

        return newResultSuccess(vm, OBJ_VAL(endRequest(vm, httpClient->curl, response, false)));
    }

    pop(vm);

    char *errorString = (char *) curl_easy_strerror(CURLE_FAILED_INIT);
    return newResultError(vm, errorString);
}

static Value httpClientHead(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "get() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "URL passed to get() must be a string.");
        return EMPTY_VAL;
    }

    HttpClient *httpClient = AS_HTTP_CLIENT(args[0]);

    CURLcode curlResponse;

    if (httpClient) {
        Response response;
        createResponse(vm, &response);
        char *url = AS_CSTRING(args[1]);

        curl_easy_setopt(httpClient->curl, CURLOPT_URL, url);
        curl_easy_setopt(httpClient->curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(httpClient->curl, CURLOPT_ACCEPT_ENCODING, "gzip");
        curl_easy_setopt(httpClient->curl, CURLOPT_HEADERDATA, &response);

        curlResponse = curl_easy_perform(httpClient->curl);

        if (curlResponse != CURLE_OK) {
            pop(vm);

            char *errorString = (char *)curl_easy_strerror(curlResponse);
            return newResultError(vm, errorString);
        }

        return newResultSuccess(vm, OBJ_VAL(endRequest(vm, httpClient->curl, response, false)));
    }

    pop(vm);

    char *errorString = (char *)curl_easy_strerror(CURLE_FAILED_INIT);
    return newResultError(vm, errorString);
}

Value newHttpClient(DictuVM *vm, ObjDict *opts) {
    ObjAbstract *abstract = newAbstract(vm, freeHttpClient, httpClientToString);
    push(vm, OBJ_VAL(abstract));

    HttpClient *httpClient = ALLOCATE(vm, HttpClient, 1);
    httpClient->curl = curl_easy_init();
    
    curl_easy_setopt(httpClient->curl, CURLOPT_HEADERFUNCTION, writeHeaders);
    curl_easy_setopt(httpClient->curl, CURLOPT_FOLLOWLOCATION, 1L);

    if (opts->count != 0) {
        for (int i = 0; i <= opts->capacityMask; i++) {
            DictItem *entry = &opts->entries[i];
            if (IS_EMPTY(entry->key)) {
                continue;
            }

            char *key;

            if (IS_STRING(entry->key)) {
                ObjString *s = AS_STRING(entry->key);
                key = s->chars;
            } else {
                runtimeError(vm, "HTTP client options key must be a string");
                return EMPTY_VAL;
            }

            if (strstr(key, "timeout")) {
                if (IS_EMPTY(entry->value)) {
                    continue;
                }
                if (!IS_NUMBER(entry->value)) {
                    runtimeError(vm, "HTTP client option \"timeout\" value must be a number");
                    return EMPTY_VAL;
                }

                curl_easy_setopt(httpClient->curl, CURLOPT_TIMEOUT, (long)AS_NUMBER(entry->value));
            } else if (strstr(key, "headers")) {
                if (IS_EMPTY(entry->value)) {
                    continue;
                }

                if (!IS_LIST(entry->value)) {
                    runtimeError(vm, "HTTP client option \"headers\" value must be a list");
                    return EMPTY_VAL;
                }

                ObjList *headers = AS_LIST(entry->value);

                for (int h = 0; h < headers->values.count; h++) {
                    headerChunk = curl_slist_append(headerChunk, AS_STRING(headers->values.values[h])->chars);
                }
                
                curl_easy_setopt(httpClient->curl, CURLOPT_HTTPHEADER, headerChunk);
            } else if (strstr(key, "insecure")) {
                if (IS_EMPTY(entry->value)) {
                    continue;
                }

                if (!IS_BOOL(entry->value)) {
                    runtimeError(vm, "HTTP client option \"insecure\" value must be a bool");
                    return EMPTY_VAL;
                }

                if (AS_BOOL(entry->value)) {
                    curl_easy_setopt(httpClient->curl, CURLOPT_SSL_VERIFYSTATUS, 0);
                    curl_easy_setopt(httpClient->curl, CURLOPT_SSL_VERIFYHOST, 0);
                    curl_easy_setopt(httpClient->curl, CURLOPT_SSL_VERIFYPEER, 0);
                }
            } else if (strstr(key, "follow_redirects")) {
                if (IS_EMPTY(entry->value)) {
                    continue;
                }

                if (!IS_BOOL(entry->value)) {
                    runtimeError(vm, "HTTP client option \"follow_redirects\" value must be a bool");
                    return EMPTY_VAL;
                }

                if (!AS_BOOL(entry->value)) {
                    curl_easy_setopt(httpClient->curl, CURLOPT_FOLLOWLOCATION, 0L);
                }
            } else if (strstr(key, "keyFile")) {
                if (IS_EMPTY(entry->value)) {
                    continue;
                }

                if (!IS_STRING(entry->value)) {
                    runtimeError(vm, "HTTP client option \"keyFile\" value must be a string");
                    return EMPTY_VAL;
                }

                char *keyFile = AS_STRING(entry->value)->chars;
                if (keyFile[0] == '\0') {
                    continue;
                }
                
                curl_easy_setopt(httpClient->curl, CURLOPT_SSLKEY, keyFile);
            } else if (strstr(key, "certFile")) {
                if (IS_EMPTY(entry->value)) {
                    continue;
                }

                if (!IS_STRING(entry->value)) {
                    runtimeError(vm, "HTTP client option \"certFile\" value must be a string");
                    return EMPTY_VAL;
                }

                char *certFile = AS_STRING(entry->value)->chars;
                if (certFile[0] == '\0') {
                    continue;
                }

                curl_easy_setopt(httpClient->curl, CURLOPT_SSLCERT, certFile);
            } else if (strstr(key, "keyPasswd")) {
                if (IS_EMPTY(entry->value)) {
                    continue;
                }

                if (!IS_STRING(entry->value)) {
                    runtimeError(vm, "HTTP client option key \"keyPasswd\" value must be a string");
                    return EMPTY_VAL;
                }

                char *keyPasswd = AS_STRING(entry->value)->chars;
                if (keyPasswd[0] == '\0') {
                    continue;
                }

                curl_easy_setopt(httpClient->curl, CURLOPT_KEYPASSWD, keyPasswd);
            }
        }
    }

    abstract->data = httpClient;

    /**
     * Setup HTTP object methods
     */
    defineNative(vm, &abstract->values, "get", httpClientGet);
    defineNative(vm, &abstract->values, "post", httpClientPost);
    defineNative(vm, &abstract->values, "put", httpClientPut);
    defineNative(vm, &abstract->values, "head", httpClientHead);
    defineNative(vm, &abstract->values, "setTimeout", httpClientSetTimeout);
    defineNative(vm, &abstract->values, "setHeaders", httpClientSetHeaders);
    defineNative(vm, &abstract->values, "setInsecure", httpClientSetInsecure);
    defineNative(vm, &abstract->values, "setFollowRedirects", httpClientSetFollowRedirects);
    defineNative(vm, &abstract->values, "setKeyFile", httpClientSetKeyFile);
    defineNative(vm, &abstract->values, "setCertFile", httpClientSetCertFile);
    defineNative(vm, &abstract->values, "setKeyPass", httpClientSetKeyPass);
    pop(vm);

    return OBJ_VAL(abstract);
}

static Value newClient(DictuVM *vm, int argCount, Value *args) {
    if (argCount > 1) {
        runtimeError(vm, "newClient() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_DICT(args[0])) {
        runtimeError(vm, "Options dict passed to newClient() must be a dict.");
        return EMPTY_VAL;
    }

    ObjDict *opts = AS_DICT(args[0]);

    return newHttpClient(vm, opts);
}

Value createHTTPModule(DictuVM *vm) {
    ObjClosure *closure = compileModuleToClosure(vm, "HTTP", DICTU_HTTP_SOURCE);

    if (closure == NULL) {
        return EMPTY_VAL;
    }

    push(vm, OBJ_VAL(closure));

    ObjModule *module = closure->function->module;

    defineNativeProperty(vm, &module->values, "METHOD_GET", OBJ_VAL(copyString(vm, HTTP_METHOD_GET, strlen(HTTP_METHOD_GET))));
    defineNativeProperty(vm, &module->values, "METHOD_POST", OBJ_VAL(copyString(vm, HTTP_METHOD_POST, strlen(HTTP_METHOD_POST))));
    defineNativeProperty(vm, &module->values, "METHOD_DELETE", OBJ_VAL(copyString(vm, HTTP_METHOD_DELETE, strlen(HTTP_METHOD_DELETE))));
    defineNativeProperty(vm, &module->values, "METHOD_PUT", OBJ_VAL(copyString(vm, HTTP_METHOD_PUT, strlen(HTTP_METHOD_PUT))));
    defineNativeProperty(vm, &module->values, "METHOD_HEAD", OBJ_VAL(copyString(vm, HTTP_METHOD_HEAD, strlen(HTTP_METHOD_HEAD))));
    defineNativeProperty(vm, &module->values, "METHOD_CONNECT", OBJ_VAL(copyString(vm, HTTP_METHOD_CONNECT, strlen(HTTP_METHOD_CONNECT))));
    defineNativeProperty(vm, &module->values, "METHOD_OPTIONS", OBJ_VAL(copyString(vm, HTTP_METHOD_OPTIONS, strlen(HTTP_METHOD_OPTIONS))));
    defineNativeProperty(vm, &module->values, "METHOD_TRACE", OBJ_VAL(copyString(vm, HTTP_METHOD_TRACE, strlen(HTTP_METHOD_TRACE))));
    defineNativeProperty(vm, &module->values, "METHOD_PATCH", OBJ_VAL(copyString(vm, HTTP_METHOD_PATCH, strlen(HTTP_METHOD_PATCH))));

    defineNativeProperty(vm, &module->values, "STATUS_CODE_CONTINUE", NUMBER_VAL(HTTP_STATUS_CODE_CONTINUE));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_SWITCHING_PROTOCOLS", NUMBER_VAL(HTTP_STATUS_CODE_SWITCHING_PROTOCOLS));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_PROCESSING", NUMBER_VAL(HTTP_STATUS_CODE_PROCESSING));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_EARLY_HINTS", NUMBER_VAL(HTTP_STATUS_CODE_EARLY_HINTS));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_OK", NUMBER_VAL(HTTP_STATUS_CODE_OK));

    defineNativeProperty(vm, &module->values, "STATUS_CODE_CREATED", NUMBER_VAL(HTTP_STATUS_CODE_CREATED));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_ACCEPTED", NUMBER_VAL(HTTP_STATUS_CODE_ACCEPTED));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_NONAUTHORITATIVE_INFO", NUMBER_VAL(HTTP_STATUS_CODE_NONAUTHORITATIVE_INFO));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_NO_CONTENT", NUMBER_VAL(HTTP_STATUS_CODE_NO_CONTENT));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_RESET_CONTENT", NUMBER_VAL(HTTP_STATUS_CODE_RESET_CONTENT));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_PARTIAL_CONTENT", NUMBER_VAL(HTTP_STATUS_CODE_PARTIAL_CONTENT));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_MULTI_STATUS", NUMBER_VAL(HTTP_STATUS_CODE_MULTI_STATUS));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_ALREADY_REPORTED", NUMBER_VAL(HTTP_STATUS_CODE_ALREADY_REPORTED));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_IM_USED", NUMBER_VAL(HTTP_STATUS_CODE_IM_USED));

    defineNativeProperty(vm, &module->values, "STATUS_CODE_MULTIPLE_CHOICES", NUMBER_VAL(HTTP_STATUS_CODE_MULTIPLE_CHOICES));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_MOVED_PERMANENTLY", NUMBER_VAL(HTTP_STATUS_CODE_MOVED_PERMANENTLY));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_FOUND", NUMBER_VAL(HTTP_STATUS_CODE_FOUND));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_SEE_OTHER", NUMBER_VAL(HTTP_STATUS_CODE_SEE_OTHER));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_NOT_MODIFIED", NUMBER_VAL(HTTP_STATUS_CODE_NOT_MODIFIED));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_USE_PROXY", NUMBER_VAL(HTTP_STATUS_CODE_USE_PROXY));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_TEMPORARY_REDIRECT", NUMBER_VAL(HTTP_STATUS_CODE_TEMPORARY_REDIRECT));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_PERMANENT_REDIRECT", NUMBER_VAL(HTTP_STATUS_CODE_PERMANENT_REDIRECT));

    defineNativeProperty(vm, &module->values, "STATUS_CODE_BAD_REQUEST", NUMBER_VAL(HTTP_STATUS_CODE_BAD_REQUEST));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_UNAUTHORIZED", NUMBER_VAL(HTTP_STATUS_CODE_UNAUTHORIZED));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_PAYMENT_REQUIRED", NUMBER_VAL(HTTP_STATUS_CODE_PAYMENT_REQUIRED));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_FORBIDDEN", NUMBER_VAL(HTTP_STATUS_CODE_FORBIDDEN));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_NOT_FOUND", NUMBER_VAL(HTTP_STATUS_CODE_NOT_FOUND));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_METHOD_NOT_ALLOWED", NUMBER_VAL(HTTP_STATUS_CODE_METHOD_NOT_ALLOWED));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_NOT_ACCEPTABLE", NUMBER_VAL(HTTP_STATUS_CODE_NOT_ACCEPTABLE));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_PROXY_AUTH_REQUIRED", NUMBER_VAL(HTTP_STATUS_CODE_PROXY_AUTH_REQUIRED));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_REQUEST_TIMEOUT", NUMBER_VAL(HTTP_STATUS_CODE_REQUEST_TIMEOUT));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_CONFLICT", NUMBER_VAL(HTTP_STATUS_CODE_CONFLICT));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_GONE", NUMBER_VAL(HTTP_STATUS_CODE_GONE));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_LENGTH_REQUIRED", NUMBER_VAL(HTTP_STATUS_CODE_LENGTH_REQUIRED));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_PRECONDITION_FAILED", NUMBER_VAL(HTTP_STATUS_CODE_PRECONDITION_FAILED));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_REQUEST_ENTITY_TOO_LARGE", NUMBER_VAL(HTTP_STATUS_CODE_REQUEST_ENTITY_TOO_LARGE));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_REQUEST_URI_TOO_LONG", NUMBER_VAL(HTTP_STATUS_CODE_REQUEST_URI_TOO_LONG));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_UNSUPPORTED_MEDIA_TYPE", NUMBER_VAL(HTTP_STATUS_CODE_UNSUPPORTED_MEDIA_TYPE));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_REQUESTED_RANGE_NOT_SATISFIABLE", NUMBER_VAL(HTTP_STATUS_CODE_REQUESTED_RANGE_NOT_SATISFIABLE));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_EXPECTATION_FAILED", NUMBER_VAL(HTTP_STATUS_CODE_EXPECTATION_FAILED));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_TEAPOT", NUMBER_VAL(HTTP_STATUS_CODE_TEAPOT));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_MISDIRECTED_REQUEST", NUMBER_VAL(HTTP_STATUS_CODE_MISDIRECTED_REQUEST));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_UNPROCESSABLE_ENTITY", NUMBER_VAL(HTTP_STATUS_CODE_UNPROCESSABLE_ENTITY));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_LOCKED", NUMBER_VAL(HTTP_STATUS_CODE_LOCKED));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_FAILED_DEPENDENCY", NUMBER_VAL(HTTP_STATUS_CODE_FAILED_DEPENDENCY));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_TOO_EARLY", NUMBER_VAL(HTTP_STATUS_CODE_TOO_EARLY));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_UPGRADE_REQUIRED", NUMBER_VAL(HTTP_STATUS_CODE_UPGRADE_REQUIRED));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_PRECONDITION_REQUIRED", NUMBER_VAL(HTTP_STATUS_CODE_PRECONDITION_REQUIRED));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_TOO_MANY_REQUESTS", NUMBER_VAL(HTTP_STATUS_CODE_TOO_MANY_REQUESTS));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE", NUMBER_VAL(HTTP_STATUS_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_UNAVAILABLE_FOR_LEGAL_REASONS", NUMBER_VAL(HTTP_STATUS_CODE_UNAVAILABLE_FOR_LEGAL_REASONS));

    defineNativeProperty(vm, &module->values, "STATUS_CODE_INTERNAL_SERVER_ERROR", NUMBER_VAL(HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_NOT_IMPLEMENTED", NUMBER_VAL(HTTP_STATUS_CODE_NOT_IMPLEMENTED));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_BAD_GATEWAY", NUMBER_VAL(HTTP_STATUS_CODE_BAD_GATEWAY));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_SERVICE_UNAVAILABLE", NUMBER_VAL(HTTP_STATUS_CODE_SERVICE_UNAVAILABLE));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_GATEWAY_TIMEOUT", NUMBER_VAL(HTTP_STATUS_CODE_GATEWAY_TIMEOUT));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_HTTP_VERSION_NOT_SUPPORTED", NUMBER_VAL(HTTP_STATUS_CODE_HTTP_VERSION_NOT_SUPPORTED));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_VARIANT_ALSO_NEGOTIATES", NUMBER_VAL(HTTP_STATUS_CODE_VARIANT_ALSO_NEGOTIATES));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_INSUFFICIENT_STORAGE", NUMBER_VAL(HTTP_STATUS_CODE_INSUFFICIENT_STORAGE));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_LOOP_DETECTED", NUMBER_VAL(HTTP_STATUS_CODE_LOOP_DETECTED));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_NOT_EXTENDED", NUMBER_VAL(HTTP_STATUS_CODE_NOT_EXTENDED));
    defineNativeProperty(vm, &module->values, "STATUS_CODE_NETWORK_AUTHENTICATION_REQUIRED", NUMBER_VAL(HTTP_STATUS_CODE_NETWORK_AUTHENTICATION_REQUIRED));

    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_CONTINUE", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_CONTINUE, strlen(HTTP_STATUS_MESSAGE_CONTINUE))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_SWITCHING_PROTOCOLS", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_SWITCHING_PROTOCOLS, strlen(HTTP_STATUS_MESSAGE_SWITCHING_PROTOCOLS))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_PROCESS", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_PROCESS, strlen(HTTP_STATUS_MESSAGE_PROCESS))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_EARLY_HINTS", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_EARLY_HINTS, strlen(HTTP_STATUS_MESSAGE_EARLY_HINTS))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_OK", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_OK, strlen(HTTP_STATUS_MESSAGE_OK))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_CREATED", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_CREATED, strlen(HTTP_STATUS_MESSAGE_CREATED))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_ACCEPTED", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_ACCEPTED, strlen(HTTP_STATUS_MESSAGE_ACCEPTED))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_NONAUTHORITATIVE_INFO", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_NONAUTHORITATIVE_INFO, strlen(HTTP_STATUS_MESSAGE_NONAUTHORITATIVE_INFO))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_NO_CONTENT", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_NO_CONTENT, strlen(HTTP_STATUS_MESSAGE_NO_CONTENT))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_RESET_CONTENT", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_RESET_CONTENT, strlen(HTTP_STATUS_MESSAGE_RESET_CONTENT))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_PARTIAL_CONTENT", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_PARTIAL_CONTENT, strlen(HTTP_STATUS_MESSAGE_PARTIAL_CONTENT))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_MULTI_STATUS", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_MULTI_STATUS, strlen(HTTP_STATUS_MESSAGE_MULTI_STATUS))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_ALREADY_REPORTED", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_ALREADY_REPORTED, strlen(HTTP_STATUS_MESSAGE_ALREADY_REPORTED))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_IM_USED", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_IM_USED, strlen(HTTP_STATUS_MESSAGE_IM_USED))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_MULTIPLE_CHOICES", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_MULTIPLE_CHOICES, strlen(HTTP_STATUS_MESSAGE_MULTIPLE_CHOICES))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_MOVED_PERMANENTLY", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_MOVED_PERMANENTLY, strlen(HTTP_STATUS_MESSAGE_MOVED_PERMANENTLY))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_FOUND", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_FOUND, strlen(HTTP_STATUS_MESSAGE_FOUND))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_SEE_OTHER", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_SEE_OTHER, strlen(HTTP_STATUS_MESSAGE_SEE_OTHER))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_NOT_MODIFIED", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_NOT_MODIFIED, strlen(HTTP_STATUS_MESSAGE_NOT_MODIFIED))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_USE_PROXY", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_USE_PROXY, strlen(HTTP_STATUS_MESSAGE_USE_PROXY))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_TEMPORARY_REDIRECT", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_TEMPORARY_REDIRECT, strlen(HTTP_STATUS_MESSAGE_TEMPORARY_REDIRECT))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_PERMANENT_REDIRECT", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_PERMANENT_REDIRECT, strlen(HTTP_STATUS_MESSAGE_PERMANENT_REDIRECT))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_BAD_REQUEST", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_BAD_REQUEST, strlen(HTTP_STATUS_MESSAGE_BAD_REQUEST))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_UNAUTHORIZED", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_UNAUTHORIZED, strlen(HTTP_STATUS_MESSAGE_UNAUTHORIZED))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_PAYMENT_REQUIRED", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_PAYMENT_REQUIRED, strlen(HTTP_STATUS_MESSAGE_PAYMENT_REQUIRED))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_FORBIDDEN", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_FORBIDDEN, strlen(HTTP_STATUS_MESSAGE_FORBIDDEN))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_NOT_FOUND", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_NOT_FOUND, strlen(HTTP_STATUS_MESSAGE_NOT_FOUND))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_METHOD_NOT_ALLOWED", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_METHOD_NOT_ALLOWED, strlen(HTTP_STATUS_MESSAGE_METHOD_NOT_ALLOWED))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_NOT_ACCEPTABLE", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_NOT_ACCEPTABLE, strlen(HTTP_STATUS_MESSAGE_NOT_ACCEPTABLE))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_PROXY_AUTH_REQUIRED", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_PROXY_AUTH_REQUIRED, strlen(HTTP_STATUS_MESSAGE_PROXY_AUTH_REQUIRED))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_REQUEST_TIMEOUT", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_REQUEST_TIMEOUT, strlen(HTTP_STATUS_MESSAGE_REQUEST_TIMEOUT))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_CONFLICT", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_CONFLICT, strlen(HTTP_STATUS_MESSAGE_CONFLICT))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_GONE", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_GONE, strlen(HTTP_STATUS_MESSAGE_GONE))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_LENGTH_REQUIRED", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_LENGTH_REQUIRED, strlen(HTTP_STATUS_MESSAGE_LENGTH_REQUIRED))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_PRECONDITION_FAILED", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_PRECONDITION_FAILED, strlen(HTTP_STATUS_MESSAGE_PRECONDITION_FAILED))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_REQUEST_ENTITY_TOO_LARGE", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_REQUEST_ENTITY_TOO_LARGE, strlen(HTTP_STATUS_MESSAGE_REQUEST_ENTITY_TOO_LARGE))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_REQUEST_URI_TOO_LONG", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_REQUEST_URI_TOO_LONG, strlen(HTTP_STATUS_MESSAGE_REQUEST_URI_TOO_LONG))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_UNSUPPORTED_MEDIA_TYPE", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_UNSUPPORTED_MEDIA_TYPE, strlen(HTTP_STATUS_MESSAGE_UNSUPPORTED_MEDIA_TYPE))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_REQUESTED_RANGE_NOT_SATISFIABLE", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_REQUESTED_RANGE_NOT_SATISFIABLE, strlen(HTTP_STATUS_MESSAGE_REQUESTED_RANGE_NOT_SATISFIABLE))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_EXPECTATION_FAILED", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_EXPECTATION_FAILED, strlen(HTTP_STATUS_MESSAGE_EXPECTATION_FAILED))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_TEAPOT", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_TEAPOT, strlen(HTTP_STATUS_MESSAGE_TEAPOT))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_MISDIRECTED_REQUEST", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_MISDIRECTED_REQUEST, strlen(HTTP_STATUS_MESSAGE_MISDIRECTED_REQUEST))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_UNPROCESSABLE_ENTITY", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_UNPROCESSABLE_ENTITY, strlen(HTTP_STATUS_MESSAGE_UNPROCESSABLE_ENTITY))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_LOCKED", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_LOCKED, strlen(HTTP_STATUS_MESSAGE_LOCKED))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_FAILED_DEPENDENCY", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_FAILED_DEPENDENCY, strlen(HTTP_STATUS_MESSAGE_FAILED_DEPENDENCY))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_TOO_EARLY", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_TOO_EARLY, strlen(HTTP_STATUS_MESSAGE_TOO_EARLY))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_UPGRADE_REQIUIRED", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_UPGRADE_REQIUIRED, strlen(HTTP_STATUS_MESSAGE_UPGRADE_REQIUIRED))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_PRECONDITION_REQUIRED", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_PRECONDITION_REQUIRED, strlen(HTTP_STATUS_MESSAGE_PRECONDITION_REQUIRED))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_TOO_MANY_REQUESTS", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_TOO_MANY_REQUESTS, strlen(HTTP_STATUS_MESSAGE_TOO_MANY_REQUESTS))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_REQUEST_HEADER_FIELDS_TOO_LARGE", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_REQUEST_HEADER_FIELDS_TOO_LARGE, strlen(HTTP_STATUS_MESSAGE_REQUEST_HEADER_FIELDS_TOO_LARGE))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_UNAVAILABLE_FOR_LEGAL_REASONS", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_UNAVAILABLE_FOR_LEGAL_REASONS, strlen(HTTP_STATUS_MESSAGE_UNAVAILABLE_FOR_LEGAL_REASONS))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_INTERNAL_SERVER_ERROR", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_INTERNAL_SERVER_ERROR, strlen(HTTP_STATUS_MESSAGE_INTERNAL_SERVER_ERROR))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_NOT_IMPLEMENTED", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_NOT_IMPLEMENTED, strlen(HTTP_STATUS_MESSAGE_NOT_IMPLEMENTED))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_BAD_GATEWAY", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_BAD_GATEWAY, strlen(HTTP_STATUS_MESSAGE_BAD_GATEWAY))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_UNAVAILABLE", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_UNAVAILABLE, strlen(HTTP_STATUS_MESSAGE_UNAVAILABLE))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_GATEWAY_TIMEOUT", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_GATEWAY_TIMEOUT, strlen(HTTP_STATUS_MESSAGE_GATEWAY_TIMEOUT))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_HTTP_VERSION_NOT_SUPPORTED", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_HTTP_VERSION_NOT_SUPPORTED, strlen(HTTP_STATUS_MESSAGE_HTTP_VERSION_NOT_SUPPORTED))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_VARIAN_ALSO_NEGOTIATES", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_VARIAN_ALSO_NEGOTIATES, strlen(HTTP_STATUS_MESSAGE_VARIAN_ALSO_NEGOTIATES))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_INSUFFICIENT_STORAGE", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_INSUFFICIENT_STORAGE, strlen(HTTP_STATUS_MESSAGE_INSUFFICIENT_STORAGE))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_LOOP_DETECTED", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_LOOP_DETECTED, strlen(HTTP_STATUS_MESSAGE_LOOP_DETECTED))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_NOT_EXTENDED", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_NOT_EXTENDED, strlen(HTTP_STATUS_MESSAGE_NOT_EXTENDED))));
    defineNativeProperty(vm, &module->values, "STATUS_MESSAGE_NETWORK_AUTHENTICATION_REQUIRED", OBJ_VAL(copyString(vm, HTTP_STATUS_MESSAGE_NETWORK_AUTHENTICATION_REQUIRED, strlen(HTTP_STATUS_MESSAGE_NETWORK_AUTHENTICATION_REQUIRED))));

    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_AIM", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_AIM, strlen(HTTP_REQUEST_HEADER_AIM))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_ACCEPT", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_ACCEPT, strlen(HTTP_REQUEST_HEADER_ACCEPT))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_ACCEPT_CHARSET", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_ACCEPT_CHARSET, strlen(HTTP_REQUEST_HEADER_ACCEPT_CHARSET))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_ACCEPT_DATETIME", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_ACCEPT_DATETIME, strlen(HTTP_REQUEST_HEADER_ACCEPT_DATETIME))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_ACCEPT_ENCODING", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_ACCEPT_ENCODING, strlen(HTTP_REQUEST_HEADER_ACCEPT_ENCODING))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_ACCEPT_LANGUAGE", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_ACCEPT_LANGUAGE, strlen(HTTP_REQUEST_HEADER_ACCEPT_LANGUAGE))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_ACCEPT_CONTROL_REQUEST_METHOD", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_ACCEPT_CONTROL_REQUEST_METHOD, strlen(HTTP_REQUEST_HEADER_ACCEPT_CONTROL_REQUEST_METHOD))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_ACCEPT_CONTROL_REQUEST_HEADERS", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_ACCEPT_CONTROL_REQUEST_HEADERS, strlen(HTTP_REQUEST_HEADER_ACCEPT_CONTROL_REQUEST_HEADERS))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_AUTHORIZATION", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_AUTHORIZATION, strlen(HTTP_REQUEST_HEADER_AUTHORIZATION))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_CACHE_CONTROL", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_CACHE_CONTROL, strlen(HTTP_REQUEST_HEADER_CACHE_CONTROL))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_CONNECTION", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_CONNECTION, strlen(HTTP_REQUEST_HEADER_CONNECTION))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_CONTENT_ENCODING", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_CONTENT_ENCODING, strlen(HTTP_REQUEST_HEADER_CONTENT_ENCODING))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_CONTENT_LENGTH", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_CONTENT_LENGTH, strlen(HTTP_REQUEST_HEADER_CONTENT_LENGTH))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_CONTENT_MD5", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_CONTENT_MD5, strlen(HTTP_REQUEST_HEADER_CONTENT_MD5))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_CONTENT_TYPE", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_CONTENT_TYPE, strlen(HTTP_REQUEST_HEADER_CONTENT_TYPE))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_COOKIE", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_COOKIE, strlen(HTTP_REQUEST_HEADER_COOKIE))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_DATE", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_DATE, strlen(HTTP_REQUEST_HEADER_DATE))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_EXPECT", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_EXPECT, strlen(HTTP_REQUEST_HEADER_EXPECT))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_FORWARDED", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_FORWARDED, strlen(HTTP_REQUEST_HEADER_FORWARDED))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_FROM", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_FROM, strlen(HTTP_REQUEST_HEADER_FROM))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_HOST", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_HOST, strlen(HTTP_REQUEST_HEADER_HOST))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_HTTP2_SETTINGS", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_HTTP2_SETTINGS, strlen(HTTP_REQUEST_HEADER_HTTP2_SETTINGS))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_IF_MATCH", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_IF_MATCH, strlen(HTTP_REQUEST_HEADER_IF_MATCH))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_IF_MODIFIED_SINCE", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_IF_MODIFIED_SINCE, strlen(HTTP_REQUEST_HEADER_IF_MODIFIED_SINCE))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_IF_NONE_MATCH", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_IF_NONE_MATCH, strlen(HTTP_REQUEST_HEADER_IF_NONE_MATCH))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_IF_RANGE", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_IF_RANGE, strlen(HTTP_REQUEST_HEADER_IF_RANGE))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_IF_UNMODIFIED_SINCE", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_IF_UNMODIFIED_SINCE, strlen(HTTP_REQUEST_HEADER_IF_UNMODIFIED_SINCE))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_MAX_FORWARDS", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_MAX_FORWARDS, strlen(HTTP_REQUEST_HEADER_MAX_FORWARDS))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_PRAGMA", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_PRAGMA, strlen(HTTP_REQUEST_HEADER_PRAGMA))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_PROXY_AUTHORIZATION", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_PROXY_AUTHORIZATION, strlen(HTTP_REQUEST_HEADER_PROXY_AUTHORIZATION))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_RANGE", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_RANGE, strlen(HTTP_REQUEST_HEADER_RANGE))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_REFERRER", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_REFERRER, strlen(HTTP_REQUEST_HEADER_REFERRER))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_TE", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_TE, strlen(HTTP_REQUEST_HEADER_TE))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_TRAILER", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_TRAILER, strlen(HTTP_REQUEST_HEADER_TRAILER))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_TRANSFER_ENCODING", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_TRANSFER_ENCODING, strlen(HTTP_REQUEST_HEADER_TRANSFER_ENCODING))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_USER_AGENT", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_USER_AGENT, strlen(HTTP_REQUEST_HEADER_USER_AGENT))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_UPGRADE", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_UPGRADE, strlen(HTTP_REQUEST_HEADER_UPGRADE))));
    defineNativeProperty(vm, &module->values, "REQUEST_HEADER_WARNING", OBJ_VAL(copyString(vm, HTTP_REQUEST_HEADER_WARNING, strlen(HTTP_REQUEST_HEADER_WARNING))));

    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, strlen(HTTP_RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_CREDENTIALS", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_CREDENTIALS, strlen(HTTP_RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_CREDENTIALS))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_ACCESS_CONTROL_EXPOSE_HEADERS", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_ACCESS_CONTROL_EXPOSE_HEADERS, strlen(HTTP_RESPONSE_HEADER_ACCESS_CONTROL_EXPOSE_HEADERS))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_ACCESS_CONTROL_MAX_AGE", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_ACCESS_CONTROL_MAX_AGE, strlen(HTTP_RESPONSE_HEADER_ACCESS_CONTROL_MAX_AGE))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_METHODS", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_METHODS, strlen(HTTP_RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_METHODS))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_HEADERS", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_HEADERS, strlen(HTTP_RESPONSE_HEADER_ACCESS_CONTROL_ALLOW_HEADERS))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_ACCEPT_PATCH", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_ACCEPT_PATCH, strlen(HTTP_RESPONSE_HEADER_ACCEPT_PATCH))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_ACCEPT_RANGES", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_ACCEPT_RANGES, strlen(HTTP_RESPONSE_HEADER_ACCEPT_RANGES))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_AGE", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_AGE, strlen(HTTP_RESPONSE_HEADER_AGE))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_ALLOW", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_ALLOW, strlen(HTTP_RESPONSE_HEADER_ALLOW))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_ALT_SVC", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_ALT_SVC, strlen(HTTP_RESPONSE_HEADER_ALT_SVC))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_CACHE_CONTROL", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_CACHE_CONTROL, strlen(HTTP_RESPONSE_HEADER_CACHE_CONTROL))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_CONNECTION", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_CONNECTION, strlen(HTTP_RESPONSE_HEADER_CONNECTION))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_CONTENT_DISPOSITION", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_CONTENT_DISPOSITION, strlen(HTTP_RESPONSE_HEADER_CONTENT_DISPOSITION))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_CONTENT_ENCODING", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_CONTENT_ENCODING, strlen(HTTP_RESPONSE_HEADER_CONTENT_ENCODING))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_CONTENT_LANGUAGE", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_CONTENT_LANGUAGE, strlen(HTTP_RESPONSE_HEADER_CONTENT_LANGUAGE))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_CONTENT_LENGTH", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_CONTENT_LENGTH, strlen(HTTP_RESPONSE_HEADER_CONTENT_LENGTH))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_CONTENT_LOCATION", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_CONTENT_LOCATION, strlen(HTTP_RESPONSE_HEADER_CONTENT_LOCATION))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_CONTENT_MD5", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_CONTENT_MD5, strlen(HTTP_RESPONSE_HEADER_CONTENT_MD5))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_CONTENT_RANGE", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_CONTENT_RANGE, strlen(HTTP_RESPONSE_HEADER_CONTENT_RANGE))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_CONTENT_TYPE", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_CONTENT_TYPE, strlen(HTTP_RESPONSE_HEADER_CONTENT_TYPE))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_DATE", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_DATE, strlen(HTTP_RESPONSE_HEADER_DATE))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_DELTA_BASE", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_DELTA_BASE, strlen(HTTP_RESPONSE_HEADER_DELTA_BASE))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_ETAG", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_ETAG, strlen(HTTP_RESPONSE_HEADER_ETAG))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_EXPIRES", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_EXPIRES, strlen(HTTP_RESPONSE_HEADER_EXPIRES))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_IM", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_IM, strlen(HTTP_RESPONSE_HEADER_IM))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_LAST_MODIFIED", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_LAST_MODIFIED, strlen(HTTP_RESPONSE_HEADER_LAST_MODIFIED))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_LINK", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_LINK, strlen(HTTP_RESPONSE_HEADER_LINK))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_LOCATION", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_LOCATION, strlen(HTTP_RESPONSE_HEADER_LOCATION))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_P3P", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_P3P, strlen(HTTP_RESPONSE_HEADER_P3P))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_PRAGMA", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_PRAGMA, strlen(HTTP_RESPONSE_HEADER_PRAGMA))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_PROXY_AUTHENTICATE", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_PROXY_AUTHENTICATE, strlen(HTTP_RESPONSE_HEADER_PROXY_AUTHENTICATE))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_PUBLIC_KEY_PINS", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_PUBLIC_KEY_PINS, strlen(HTTP_RESPONSE_HEADER_PUBLIC_KEY_PINS))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_RETRY_AFTER", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_RETRY_AFTER, strlen(HTTP_RESPONSE_HEADER_RETRY_AFTER))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_SET_COOKIE", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_SET_COOKIE, strlen(HTTP_RESPONSE_HEADER_SET_COOKIE))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_STRICT_TRANSPORT_SECURITY", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_STRICT_TRANSPORT_SECURITY, strlen(HTTP_RESPONSE_HEADER_STRICT_TRANSPORT_SECURITY))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_TRAILER", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_TRAILER, strlen(HTTP_RESPONSE_HEADER_TRAILER))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_TRANSFER_ENCODING", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_TRANSFER_ENCODING, strlen(HTTP_RESPONSE_HEADER_TRANSFER_ENCODING))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_TK", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_TK, strlen(HTTP_RESPONSE_HEADER_TK))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_UPGRADE", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_UPGRADE, strlen(HTTP_RESPONSE_HEADER_UPGRADE))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_VARY", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_VARY, strlen(HTTP_RESPONSE_HEADER_VARY))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_VIA", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_VIA, strlen(HTTP_RESPONSE_HEADER_VIA))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_WARNING", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_WARNING, strlen(HTTP_RESPONSE_HEADER_WARNING))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_WWW_AUTHENTICATE", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_WWW_AUTHENTICATE, strlen(HTTP_RESPONSE_HEADER_WWW_AUTHENTICATE))));
    defineNativeProperty(vm, &module->values, "RESPONSE_HEADER_X_FRAME_OPTIONS", OBJ_VAL(copyString(vm, HTTP_RESPONSE_HEADER_X_FRAME_OPTIONS, strlen(HTTP_RESPONSE_HEADER_X_FRAME_OPTIONS))));

    defineNativeProperty(vm, &module->values, "MAX_HEADER_BYTES", AS_NUMBER(HTTP_MAX_HEADER_BYTES));

    /**
     * Define Http methods
     */
    defineNative(vm, &module->values, "get", get);
    defineNative(vm, &module->values, "post", post);
    defineNative(vm, &module->values, "put", put);
    defineNative(vm, &module->values, "head", head);

    defineNative(vm, &module->values, "newClient", newClient);

    pop(vm);

    return OBJ_VAL(closure);
}
