#include "http.h"

static Value strerrorHttpNative(VM *vm, int argCount, Value *args) {
    if (argCount > 1) {
        runtimeError(vm, "strerror() takes either 0 or 1 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    int error;
    if (argCount == 1) {
        error = AS_NUMBER(args[0]);
    } else {
        error = AS_NUMBER(getErrno(vm, GET_SELF_CLASS));
    }

    char *error_string = (char *) curl_easy_strerror(error);
    return OBJ_VAL(copyString(vm, error_string, strlen(error_string)));
}

static void createResponse(VM *vm, Response *response) {
    response->vm = vm;
    response->headers = initList(vm);
    // Push to stack to avoid GC
    push(vm, OBJ_VAL(response->headers));

    response->len = 0;
    response->res = NULL;
}

static size_t writeResponse(char *ptr, size_t size, size_t nmemb, Response *response) {
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

static ObjDict *endRequest(VM *vm, CURL *curl, Response response) {
    // Get status code
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.statusCode);
    ObjString *content = takeString(vm, response.res, response.len);

    // Push to stack to avoid GC
    push(vm, OBJ_VAL(content));

    ObjDict *responseVal = initDict(vm);
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

static Value get(VM *vm, int argCount, Value *args) {
    if (argCount != 1 && argCount != 2) {
        runtimeError(vm, "get() takes 1 or 2 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    long timeout = 20;

    if (argCount == 2) {
        if (!IS_NUMBER(args[1])) {
            runtimeError(vm, "Timeout passed to get() must be a number.");
            return EMPTY_VAL;
        }

        timeout = AS_NUMBER(args[1]);
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

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeResponse);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writeHeaders);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response);

        /* Perform the request, res will get the return code */
        curlResponse = curl_easy_perform(curl);

        /* Check for errors */
        if (curlResponse != CURLE_OK) {
            /* always cleanup */
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            pop(vm);

            errno = curlResponse;
            SET_ERRNO(GET_SELF_CLASS);
            return NIL_VAL;
        }

        return OBJ_VAL(endRequest(vm, curl, response));
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    pop(vm);

    errno = CURLE_FAILED_INIT;
    SET_ERRNO(GET_SELF_CLASS);
    return NIL_VAL;
}

static Value post(VM *vm, int argCount, Value *args) {
    if (argCount != 1 && argCount != 2 && argCount != 3) {
        runtimeError(vm, "post() takes between 1 and 3 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    long timeout = 20;
    ObjDict *dict = NULL;

    if (argCount == 3) {
        if (!IS_NUMBER(args[2])) {
            runtimeError(vm, "Timeout passed to post() must be a number.");
            return EMPTY_VAL;
        }

        if (!IS_DICT(args[1])) {
            runtimeError(vm, "Post values passed to post() must be a dictionary.");
            return EMPTY_VAL;
        }

        timeout = (long) AS_NUMBER(args[2]);
        dict = AS_DICT(args[1]);
    } else if (argCount == 2) {
        if (!IS_DICT(args[1])) {
            runtimeError(vm, "Post values passed to post() must be a dictionary.");
            return EMPTY_VAL;
        }

        dict = AS_DICT(args[1]);
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

        if (dict != NULL) {
            postValue = dictToPostArgs(dict);
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postValue);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeResponse);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writeHeaders);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response);

        /* Perform the request, res will get the return code */
        curlResponse = curl_easy_perform(curl);

        if (dict != NULL) {
            free(postValue);
        }

        if (curlResponse != CURLE_OK) {
            /* always cleanup */
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            pop(vm);

            errno = curlResponse;
            SET_ERRNO(GET_SELF_CLASS);
            return NIL_VAL;
        }

        return OBJ_VAL(endRequest(vm, curl, response));
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    pop(vm);

    errno = CURLE_FAILED_INIT;
    SET_ERRNO(GET_SELF_CLASS);
    return NIL_VAL;
}

ObjModule *createHTTPModule(VM *vm) {
    ObjString *name = copyString(vm, "HTTP", 4);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Http methods
     */
    defineNative(vm, &module->values, "strerror", strerrorHttpNative);
    defineNative(vm, &module->values, "get", get);
    defineNative(vm, &module->values, "post", post);

    /**
     * Define Http properties
     */
    defineNativeProperty(vm, &module->values, "errno", NUMBER_VAL(0));
    pop(vm);
    pop(vm);

    return module;
}
