#include "http.h"

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

static ObjDict *endRequest(DictuVM *vm, CURL *curl, Response response) {
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

    ObjDict *responseVal = newDict(vm);
    // Push to stack to avoid GC
    push(vm, OBJ_VAL(responseVal));

    ObjString *string = copyString(vm, "content", 7);
    push(vm, OBJ_VAL(string));
    dictSet(vm, responseVal, OBJ_VAL(string), OBJ_VAL(content));
    pop(vm);

    string = copyString(vm, "headers", 7);
    push(vm, OBJ_VAL(string));
    dictSet(vm, responseVal, OBJ_VAL(string), OBJ_VAL(response.headers));
    pop(vm);

    string = copyString(vm, "statusCode", 10);
    push(vm, OBJ_VAL(string));
    dictSet(vm, responseVal, OBJ_VAL(string), NUMBER_VAL(response.statusCode));
    pop(vm);

    // Pop
    pop(vm);
    pop(vm);
    pop(vm);

    /* always cleanup */
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return responseVal;
}

static Value get(DictuVM *vm, int argCount, Value *args) {
    if (argCount < 0 || argCount > 3) {
        runtimeError(vm, "get() takes between 1 and 3 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    long timeout = 20;
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

        return newResultSuccess(vm, OBJ_VAL(endRequest(vm, curl, response)));
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

    long timeout = 20;
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

        return newResultSuccess(vm, OBJ_VAL(endRequest(vm, curl, response)));
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    pop(vm);

    char *errorString = (char *) curl_easy_strerror(CURLE_FAILED_INIT);
    return newResultError(vm, errorString);
}

ObjModule *createHTTPModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "HTTP", 4);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Http methods
     */
    defineNative(vm, &module->values, "get", get);
    defineNative(vm, &module->values, "post", post);

    pop(vm);
    pop(vm);

    return module;
}
