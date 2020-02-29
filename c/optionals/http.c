#include "http.h"

static void createResponse(Response *response) {
    response->headers = initList();
    // Push to stack to avoid GC
    push(OBJ_VAL(response->headers));

    response->len = 0;
    response->res = malloc(response->len + 1);
    if (response->res == NULL) {
        printf("Unable to allocate memory\n");
        exit(71);
    }
    response->res[0] = '\0';
}

static size_t writeResponse(char *ptr, size_t size, size_t nmemb, void *data)
{
    Response *response = (Response *) data;
    size_t new_len = response->len + size * nmemb;
    response->res = realloc(response->res, new_len + 1);
    if (response->res == NULL) {
        printf("Unable to allocate memory\n");
        exit(71);
    }
    memcpy(response->res + response->len, ptr, size * nmemb);
    response->res[new_len] = '\0';
    response->len = new_len;

    return size * nmemb;
}

static size_t writeHeaders(char *ptr, size_t size, size_t nitems, void *data)
{
    Response *response = (Response *) data;
    // if nitems equals 2 its an empty header
    if (nitems != 2) {
        Value header = OBJ_VAL(copyString(ptr, (nitems - 2) * size));
        // Push to stack to avoid GC
        push(header);
        writeValueArray(&response->headers->values, header);
        pop();
    }
    return size * nitems;
}

static char *dictToPostArgs(ObjDict *dict) {
    int len = 100;
    char *ret = malloc(sizeof(char) * len);
    int currentLen = 0;

    for (int i = 0; i < dict->capacity; i++) {
        if (dict->items[i] == NULL) {
            continue;
        }

        char *value;
        if (IS_STRING(dict->items[i]->item)) {
            value = AS_CSTRING(dict->items[i]->item);
        } else {
            value = valueToString(dict->items[i]->item);
        }

        int keyLen = strlen(dict->items[i]->key);
        int valLen = strlen(value);

        if (currentLen + keyLen + valLen > len) {
            len = len * 2 + keyLen + valLen;
            ret = realloc(ret, len);

            if (ret == NULL) {
                printf("Unable to allocate memory\n");
                exit(71);
            }
        }

        memcpy(ret + currentLen, dict->items[i]->key, keyLen);
        currentLen += keyLen;
        memcpy(ret + currentLen, "=", 1);
        currentLen += 1;
        memcpy(ret + currentLen, value, valLen);
        currentLen += valLen;
        memcpy(ret + currentLen, "&", 1);
        currentLen += 1;

        if (!IS_STRING(dict->items[i]->item)) {
            free(value);
        }
    }

    ret[currentLen] = '\0';

    return ret;
}

static Value get(int argCount, Value *args) {
    if (argCount != 1 && argCount != 2) {
        runtimeError("get() takes 1 or 2 arguments (%d  given)", argCount);
        return EMPTY_VAL;
    }

    long timeout = 20;

    if (argCount == 2) {
        if (!IS_NUMBER(args[1])) {
            runtimeError("Timeout passed to get() must be a number");
            return EMPTY_VAL;
        }

        timeout = AS_NUMBER(args[1]);
    }

    if (!IS_STRING(args[0])) {
        runtimeError("URL passed to get() must be a string");
        return EMPTY_VAL;
    }

    CURL *curl;
    CURLcode curlResponse;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        Response response;
        createResponse(&response);
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
            runtimeError("cURL request failed: %s", curl_easy_strerror(curlResponse));
            return EMPTY_VAL;
        }

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.statusCode);

        ObjString *content = copyString(response.res, response.len);
        free(response.res);
        // Push to stack to avoid GC
        push(OBJ_VAL(content));

        /* always cleanup */
        curl_easy_cleanup(curl);

        curl_global_cleanup();

        ObjDict *responseVal = initDict();
        // Push to stack to avoid GC
        push(OBJ_VAL(responseVal));

        insertDict(responseVal, "content", OBJ_VAL(content));
        insertDict(responseVal, "headers", OBJ_VAL(response.headers));
        insertDict(responseVal, "statusCode", NUMBER_VAL(response.statusCode));

        // Pop header list, content and response dict return off stack
        pop();
        pop();
        pop();

        return OBJ_VAL(responseVal);
    }

    runtimeError("cURL failed to initialise");
    return EMPTY_VAL;
}

static Value post(int argCount, Value *args) {
    if (argCount != 1 && argCount != 2 && argCount != 3) {
        runtimeError("post() takes between 1 and 3 arguments (%d  given)", argCount);
        return EMPTY_VAL;
    }

    long timeout = 20;
    ObjDict *dict = NULL;

    if (argCount == 3) {
        if (!IS_NUMBER(args[2])) {
            runtimeError("Timeout passed to post() must be a number");
            return EMPTY_VAL;
        }

        if (!IS_DICT(args[1])) {
            runtimeError("Post values passed to post() must be a dictionary");
            return EMPTY_VAL;
        }

        timeout = (long)AS_NUMBER(args[2]);
        dict = AS_DICT(args[1]);
    } else if (argCount == 2) {
        if (!IS_DICT(args[1])) {
            runtimeError("Post values passed to post() must be a dictionary");
            return EMPTY_VAL;
        }

        dict = AS_DICT(args[1]);
    }

    if (!IS_STRING(args[0])) {
        runtimeError("URL passed to post() must be a string");
        return EMPTY_VAL;
    }

    CURL *curl;
    CURLcode curlResponse;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        char *url = AS_CSTRING(args[0]);
        char *postValue = "";
        Response response;
        createResponse(&response);

        if (dict != NULL) {
            postValue = dictToPostArgs(dict);
        }

        // Set cURL options
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postValue);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeResponse);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writeHeaders);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response);

        /* Perform the request, res will get the return code */
        curlResponse = curl_easy_perform(curl);

        /* Check for errors */
        if (curlResponse != CURLE_OK) {
            runtimeError("cURL request failed: %s", curl_easy_strerror(curlResponse));
            return EMPTY_VAL;
        }

        // Get status code
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.statusCode);

        ObjString *content = copyString(response.res, response.len);
        free(response.res);
        // Push to stack to avoid GC
        push(OBJ_VAL(content));

        if (dict != NULL) {
            free(postValue);
        }

        /* always cleanup */
        curl_easy_cleanup(curl);

        curl_global_cleanup();

        ObjDict *responseVal = initDict();
        // Push to stack to avoid GC
        push(OBJ_VAL(responseVal));

        insertDict(responseVal, "content", OBJ_VAL(content));
        insertDict(responseVal, "headers", OBJ_VAL(response.headers));
        insertDict(responseVal, "statusCode", NUMBER_VAL(response.statusCode));

        // Pop header list and dict return off stack
        pop();
        pop();
        pop();

        return OBJ_VAL(responseVal);
    }

    runtimeError("cURL failed to initialise");
    return EMPTY_VAL;
}

void createHTTPClass() {
    ObjString *name = copyString("HTTP", 4);
    push(OBJ_VAL(name));
    ObjClassNative *klass = newClassNative(name);
    push(OBJ_VAL(klass));

    /**
     * Define Http methods
     */
    defineNativeMethod(klass, "get", get);
    defineNativeMethod(klass, "post", post);

    tableSet(&vm.globals, name, OBJ_VAL(klass));
    pop();
    pop();
}