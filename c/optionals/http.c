#include "http.h"

static void createResponse(VM *vm, Response *response) {
    printf("wowow!\n");
    response->vm = vm;
    response->headers = initList(vm);
    // Push to stack to avoid GC
    push(vm, OBJ_VAL(response->headers));
    printf("wowow1!\n");

    response->len = 0;
    response->res = malloc(response->len + 1);
    if (response->res == NULL) {
        printf("Unable to allocate memory\n");
        exit(71);
    }
    response->res[0] = '\0';
    printf("wowow2!\n");
}

static size_t writeResponse(char *ptr, size_t size, size_t nmemb, void *data)
{
    printf("ok!\n");
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
    printf("ok1!\n");

    return size * nmemb;
}

static size_t writeHeaders(char *ptr, size_t size, size_t nitems, void *data)
{
    printf("head!\n");
    Response *response = (Response *) data;
    printf("head1.0!\n");
    // if nitems equals 2 its an empty header
    if (nitems != 2) {
        printf("head1.01!\n");
        Value header = OBJ_VAL(copyString(response->vm, ptr, (nitems - 2) * size));
        printf("head1.1!\n");
        // Push to stack to avoid GC
        push(response->vm, header);
        printf("head1.2!\n");
        writeValueArray(response->vm, &response->headers->values, header);
        printf("head1.3!\n");
        pop(response->vm);
        printf("head1.4!\n");
    }
    printf("head1!\n");
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

static Value get(VM *vm, int argCount, Value *args) {
    if (argCount != 1 && argCount != 2) {
        runtimeError(vm, "get() takes 1 or 2 arguments (%d  given)", argCount);
        return EMPTY_VAL;
    }

    long timeout = 20;

    if (argCount == 2) {
        if (!IS_NUMBER(args[1])) {
            runtimeError(vm, "Timeout passed to get() must be a number");
            return EMPTY_VAL;
        }

        timeout = AS_NUMBER(args[1]);
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "URL passed to get() must be a string");
        return EMPTY_VAL;
    }

    CURL *curl;
    CURLcode curlResponse;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        printf("here\n");
        Response response;
        createResponse(vm, &response);
        printf("here1.11\n");
        char *url = AS_CSTRING(args[0]);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeResponse);
        printf("here1.111\n");
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writeHeaders);
        printf("here1.1111\n");
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response);
        printf("here1.11111\n");

        /* Perform the request, res will get the return code */
        curlResponse = curl_easy_perform(curl);
        printf("here1\n");

        /* Check for errors */
        if (curlResponse != CURLE_OK) {
            runtimeError(vm, "cURL request failed: %s", curl_easy_strerror(curlResponse));
            return EMPTY_VAL;
        }

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.statusCode);
        printf("here1.1\n");

        ObjString *content = copyString(vm, response.res, response.len);
        printf("here1.2\n");
        free(response.res);

        printf("here2\n");
        // Push to stack to avoid GC
        push(vm, OBJ_VAL(content));

        /* always cleanup */
        curl_easy_cleanup(curl);

        curl_global_cleanup();

        printf("here3\n");

        ObjDict *responseVal = initDict(vm);
        // Push to stack to avoid GC
        push(vm, OBJ_VAL(responseVal));

        printf("here4\n");

        insertDict(vm, responseVal, "content", OBJ_VAL(content));
        insertDict(vm, responseVal, "headers", OBJ_VAL(response.headers));
        insertDict(vm, responseVal, "statusCode", NUMBER_VAL(response.statusCode));
        printf("here5\n");

        // Pop header list, content and response dict return off stack
        pop(vm);
        pop(vm);
        pop(vm);

        return OBJ_VAL(responseVal);
    }

    runtimeError(vm, "cURL failed to initialise");
    return EMPTY_VAL;
}

static Value post(VM *vm, int argCount, Value *args) {
    if (argCount != 1 && argCount != 2 && argCount != 3) {
        runtimeError(vm, "post() takes between 1 and 3 arguments (%d  given)", argCount);
        return EMPTY_VAL;
    }

    long timeout = 20;
    ObjDict *dict = NULL;

    if (argCount == 3) {
        if (!IS_NUMBER(args[2])) {
            runtimeError(vm, "Timeout passed to post() must be a number");
            return EMPTY_VAL;
        }

        if (!IS_DICT(args[1])) {
            runtimeError(vm, "Post values passed to post() must be a dictionary");
            return EMPTY_VAL;
        }

        timeout = (long)AS_NUMBER(args[2]);
        dict = AS_DICT(args[1]);
    } else if (argCount == 2) {
        if (!IS_DICT(args[1])) {
            runtimeError(vm, "Post values passed to post() must be a dictionary");
            return EMPTY_VAL;
        }

        dict = AS_DICT(args[1]);
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "URL passed to post() must be a string");
        return EMPTY_VAL;
    }

    CURL *curl;
    CURLcode curlResponse;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        printf("return\n");
        char *url = AS_CSTRING(args[0]);
        char *postValue = "";
        Response response;
        createResponse(vm, &response);
        printf("return1\n");

        if (dict != NULL) {
            postValue = dictToPostArgs(dict);
        }
        printf("return2\n");

        // Set cURL options
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postValue);
        printf("return3\n");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeResponse);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writeHeaders);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response);

        /* Perform the request, res will get the return code */
        curlResponse = curl_easy_perform(curl);
        printf("return4\n");

        /* Check for errors */
        if (curlResponse != CURLE_OK) {
            runtimeError(vm, "cURL request failed: %s", curl_easy_strerror(curlResponse));
            return EMPTY_VAL;
        }

        // Get status code
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.statusCode);

        printf("content?????\n");
        ObjString *content = copyString(vm, response.res, response.len);
        printf("content1?????\n");
        free(response.res);
        // Push to stack to avoid GC
        push(vm, OBJ_VAL(content));

        if (dict != NULL) {
            free(postValue);
        }
        printf("return5\n");

        /* always cleanup */
        curl_easy_cleanup(curl);

        curl_global_cleanup();

        ObjDict *responseVal = initDict(vm);
        printf("return6\n");
        // Push to stack to avoid GC
        push(vm, OBJ_VAL(responseVal));

        insertDict(vm, responseVal, "content", OBJ_VAL(content));
        insertDict(vm, responseVal, "headers", OBJ_VAL(response.headers));
        insertDict(vm, responseVal, "statusCode", NUMBER_VAL(response.statusCode));

        // Pop header list and dict return off stack
        pop(vm);
        pop(vm);
        pop(vm);
        printf("return90\n");

        return OBJ_VAL(responseVal);
        // return NIL_VAL;
    }

    runtimeError(vm, "cURL failed to initialise");
    return EMPTY_VAL;
}

void createHTTPClass(VM *vm) {
    printf("HTTP??\n");
    ObjString *name = copyString(vm, "HTTP", 4);
    printf("HTTP1??\n");
    push(vm, OBJ_VAL(name));
    ObjClassNative *klass = newClassNative(vm, name);
    push(vm, OBJ_VAL(klass));

    /**
     * Define Http methods
     */
    defineNativeMethod(vm, klass, "get", get);
    defineNativeMethod(vm, klass, "post", post);

    tableSet(vm, &vm->globals, name, OBJ_VAL(klass));
    pop(vm);
    pop(vm);
}