#include "json.h"

struct json_error_table_t {
  int error;
  const char *description;
} json_error_table [] = {
#define JSON_ENULL 1
  { JSON_ENULL, "Json object value is nil"},
#define JSON_ENOTYPE 2
  { JSON_ENOTYPE, "No such type"},
#define JSON_EINVAL 3
  { JSON_EINVAL, "Invalid JSON object"},
#define JSON_ENOSERIAL 4
  { JSON_ENOSERIAL, "Object is not serializable"},
  { -1, NULL}};

static Value strerrorJsonNative(VM *vm, int argCount, Value *args) {
    if (argCount > 1) {
        runtimeError(vm, "strerror() takes either 0 or 1 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    int error;
    if (argCount == 1) {
        error = AS_NUMBER(args[0]);
    } else {
        error = AS_NUMBER(GET_ERRNO(GET_SELF_CLASS));
    }

    if (error == 0) {
        return OBJ_VAL(copyString(vm, "", 0));
    }

    if (error < 0) {
        runtimeError(vm, "strerror() argument should be greater than or equal to 0");
        return EMPTY_VAL;
    }

    for (int i = 0; json_error_table[i].error != -1; i++) {
        if (error == json_error_table[i].error) {
            return OBJ_VAL(copyString(vm, json_error_table[i].description,
                strlen (json_error_table[i].description)));
        }
    }

    runtimeError(vm, "strerror() argument should be <= %d", JSON_ENOSERIAL);
    return EMPTY_VAL;
}

static Value parseJson(VM *vm, json_value *json) {
    switch (json->type) {
        case json_none:
        case json_null: {
            // TODO: We return nil on failure however "null" is valid JSON
            // TODO: We need a better way of handling this scenario
            return NIL_VAL;
        }

        case json_object: {
            ObjDict *dict = initDict(vm);
            // Push value to stack to avoid GC
            push(vm, OBJ_VAL(dict));

            for (unsigned int i = 0; i < json->u.object.length; i++) {
                Value val = parseJson(vm, json->u.object.values[i].value);
                push(vm, val);
                dictSet(vm, dict, OBJ_VAL(copyString(vm, json->u.object.values[i].name, json->u.object.values[i].name_length)), val);
                pop(vm);
            }

            pop(vm);

            return OBJ_VAL(dict);
        }

        case json_array: {
            ObjList *list = initList(vm);
            // Push value to stack to avoid GC
            push(vm, OBJ_VAL(list));

            for (unsigned int i = 0; i < json->u.array.length; i++) {
                Value val = parseJson(vm, json->u.array.values[i]);
                push(vm, val);
                writeValueArray(vm, &list->values, val);
                pop(vm);
            }

            // Pop list
            pop(vm);

            return OBJ_VAL(list);
        }

        case json_integer: {
            return NUMBER_VAL(json->u.integer);
        }

        case json_double: {
            return NUMBER_VAL(json->u.dbl);
        }

        case json_string: {
            return OBJ_VAL(copyString(vm, json->u.string.ptr, json->u.string.length));
        }

        case json_boolean: {
            return BOOL_VAL(json->u.boolean);
        }

        default: {
            errno = JSON_ENOTYPE;
            return EMPTY_VAL;
        }
    }
}

static Value parse(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "parse() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "parse() argument must be a string.");
        return EMPTY_VAL;
    }

    ObjString *json = AS_STRING(args[0]);
    json_value *json_obj = json_parse(json->chars, json->length);

    if (json_obj == NULL) {
        errno = JSON_EINVAL;
        SET_ERRNO(GET_SELF_CLASS);
        return NIL_VAL;
    }

    Value val = parseJson(vm, json_obj);

    if (val == EMPTY_VAL) {
        SET_ERRNO(GET_SELF_CLASS);
        return NIL_VAL;
    }

    json_value_free(json_obj);

    return val;
}

json_value* stringifyJson(Value value) {
    if (IS_NIL(value)) {
        return json_null_new();
    } else if (IS_BOOL(value)) {
        return json_boolean_new(AS_BOOL(value));
    } else if (IS_NUMBER(value)) {
        double num = AS_NUMBER(value);

        if ((int) num == num) {
            return json_integer_new((int) num);
        }

        return json_double_new(num);
    } else if (IS_OBJ(value)) {
        switch (AS_OBJ(value)->type) {
            case OBJ_STRING: {
                return json_string_new(AS_CSTRING(value));
            }

            case OBJ_LIST: {
                ObjList *list = AS_LIST(value);
                json_value *json = json_array_new(list->values.count);

                for (int i = 0; i < list->values.count; i++) {
                    json_array_push(json, stringifyJson(list->values.values[i]));
                }

                return json;
            }

            case OBJ_DICT: {
                ObjDict *dict = AS_DICT(value);
                json_value *json = json_object_new(dict->count);

                for (int i = 0; i <= dict->capacityMask; i++) {
                    DictItem *entry = &dict->entries[i];
                    if (IS_EMPTY(entry->key)) {
                        continue;
                    }

                    char *key;

                    if (IS_STRING(entry->key)) {
                        ObjString *s = AS_STRING(entry->key);
                        key = s->chars;
                    } else if (IS_NIL(entry->key)) {
                        key = malloc(5);
                        memcpy(key, "null", 4);
                        key[4] = '\0';
                    } else {
                        key = valueToString(entry->key);
                    }

                    json_object_push(
                            json,
                            key,
                            stringifyJson(entry->value)
                    );

                    if (!IS_STRING(entry->key)) {
                        free(key);
                    }
                }

                return json;
            }

            // Pass through and return NULL
            default: {}
        }
    }

    return NULL;
}

static Value stringify(VM *vm, int argCount, Value *args) {
    if (argCount != 1 && argCount != 2) {
        runtimeError(vm, "stringify() takes 1 or 2 arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    int indent = 4;
    int lineType = json_serialize_mode_single_line;

    if (argCount == 2) {
        if (!IS_NUMBER(args[1])) {
            runtimeError(vm, "stringify() second argument must be a number.");
            return EMPTY_VAL;
        }

        lineType = json_serialize_mode_multiline;
        indent = AS_NUMBER(args[1]);
    }

    json_value *json = stringifyJson(args[0]);

    if (json == NULL) {
        errno = JSON_ENOSERIAL;
        SET_ERRNO(GET_SELF_CLASS);
        return NIL_VAL;
    }

    json_serialize_opts default_opts =
    {
        lineType,
        json_serialize_opt_pack_brackets,
        indent
    };

    char *buf = malloc(json_measure_ex(json, default_opts));
    json_serialize_ex(buf, json, default_opts);
    ObjString *string = copyString(vm, buf, strlen(buf));
    free(buf);
    json_builder_free(json);
    return OBJ_VAL(string);
}

ObjModule *createJSONClass(VM *vm) {
    ObjString *name = copyString(vm, "JSON", 4);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Json methods
     */
    defineNative(vm, &module->values, "strerror", strerrorJsonNative);
    defineNative(vm, &module->values, "parse", parse);
    defineNative(vm, &module->values, "stringify", stringify);

    /**
     * Define Json properties
     */
    defineNativeProperty(vm, &module->values, "errno", NUMBER_VAL(0));
    defineNativeProperty(vm, &module->values, "ENULL", NUMBER_VAL(JSON_ENULL));
    defineNativeProperty(vm, &module->values, "ENOTYPE", NUMBER_VAL(JSON_ENOTYPE));
    defineNativeProperty(vm, &module->values, "EINVAL", NUMBER_VAL(JSON_EINVAL));
    defineNativeProperty(vm, &module->values, "ENOSERIAL", NUMBER_VAL(JSON_ENOSERIAL));
    pop(vm);
    pop(vm);

    return module;
}
