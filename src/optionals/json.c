#include "json.h"

static Value parseJson(DictuVM *vm, json_value *json) {
    switch (json->type) {
        case json_none:
        case json_null: {
            return NIL_VAL;
        }

        case json_object: {
            ObjDict *dict = newDict(vm);
            // Push value to stack to avoid GC
            push(vm, OBJ_VAL(dict));

            for (unsigned int i = 0; i < json->u.object.length; i++) {
                Value val = parseJson(vm, json->u.object.values[i].value);
                push(vm, val);
                Value key = OBJ_VAL(copyString(vm, json->u.object.values[i].name, json->u.object.values[i].name_length));
                push(vm, key);
                dictSet(vm, dict, key, val);
                pop(vm);
                pop(vm);
            }

            pop(vm);

            return OBJ_VAL(dict);
        }

        case json_array: {
            ObjList *list = newList(vm);
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
            return EMPTY_VAL;
        }
    }
}

static Value parse(DictuVM *vm, int argCount, Value *args) {
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
        return newResultError(vm, "Invalid JSON object");
    }

    Value val = parseJson(vm, json_obj);

    if (val == EMPTY_VAL) {
        return newResultError(vm, "Invalid JSON object");
    }

    json_value_free(json_obj);

    return newResultSuccess(vm, val);
}

json_value* stringifyJson(DictuVM *vm, Value value) {
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
                    json_array_push(json, stringifyJson(vm, list->values.values[i]));
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
                        key = valueToString(vm, entry->key);
                    }

                    json_object_push(
                        json,
                        key,
                        stringifyJson(vm, entry->value)
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

static Value stringify(DictuVM *vm, int argCount, Value *args) {
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

    json_value *json = stringifyJson(vm, args[0]);

    if (json == NULL) {
        return newResultError(vm, "Object is not serializable");
    }

    json_serialize_opts default_opts =
    {
        lineType,
        json_serialize_opt_pack_brackets,
        indent
    };


    int length = json_measure_ex(json, default_opts);
    char *buf = ALLOCATE(vm, char, length);
    json_serialize_ex(buf, json, default_opts);
    int actualLength = strlen(buf);

    // json_measure_ex can produce a length larger than the actual string returned
    // so we need to cater for this case
    if (actualLength != length) {
        buf = SHRINK_ARRAY(vm, buf, char, length, actualLength + 1);
    }

    ObjString *string = takeString(vm, buf, actualLength);
    json_builder_free(json);

    return newResultSuccess(vm, OBJ_VAL(string));
}

Value createJSONModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "JSON", 4);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Json methods
     */
    defineNative(vm, &module->values, "parse", parse);
    defineNative(vm, &module->values, "stringify", stringify);

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
