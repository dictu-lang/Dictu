#include "json.h"

static Value parseJson(VM *vm, json_value *json) {
    switch (json->type) {
        case json_none:
        case json_null: {
            return NIL_VAL;
        }

        case json_object: {
            ObjDict *dict = initDict(vm);
            // Push value to stack to avoid GC
            push(vm, OBJ_VAL(dict));

            for (unsigned int i = 0; i < json->u.object.length; i++) {
                Value val = parseJson(vm, json->u.object.values[i].value);
                push(vm, val);
                insertDict(vm, dict, json->u.object.values[i].name, val);
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
            return EMPTY_VAL;
        }
    }
}

static Value parse(VM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "parse() takes 1 argument (%d  given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "parse() argument must be a string.");
        return EMPTY_VAL;
    }

    ObjString *json = AS_STRING(args[0]);
    json_value *json_obj = json_parse(json->chars, json->length);

    if (json_obj == NULL) {
        runtimeError(vm, "Invalid JSON passed to parse()");
        return EMPTY_VAL;
    }

    Value val = parseJson(vm, json_obj);

    if (val == EMPTY_VAL) {
        runtimeError(vm, "Invalid JSON passed to parse()");
        return EMPTY_VAL;
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

                for (int i = 0; i < dict->capacity; i++) {
                    if (dict->items[i] == NULL) {
                        continue;
                    }

                    json_object_push_nocopy(json, strlen(dict->items[i]->key), dict->items[i]->key, stringifyJson(dict->items[i]->item));
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
        runtimeError(vm, "stringify() takes 1 or 2 arguments (%d  given)", argCount);
        return EMPTY_VAL;
    }

    int indent = 4;
    int lineType = json_serialize_mode_single_line;

    if (argCount == 2) {
        if (!IS_NUMBER(args[1])) {
            runtimeError(vm, "stringify() second argument must be a number");
            return EMPTY_VAL;
        }

        lineType = json_serialize_mode_multiline;
        indent = AS_NUMBER(args[1]);
    }

    json_value *json = stringifyJson(args[0]);

    if (json == NULL) {
        runtimeError(vm, "Value: %s is not JSON serializable", valueToString(args[0]));
        return EMPTY_VAL;
    }

    json_serialize_opts default_opts =
    {
            lineType,
            json_serialize_opt_pack_brackets,
            indent
    };


    char *buf = malloc(json_measure(json));
    json_serialize_ex(buf, json, default_opts);
    ObjString *string = copyString(vm, buf, strlen(buf));
    free(buf);
    json_value_free(json);
    return OBJ_VAL(string);
}

void createJSONClass(VM *vm) {
    ObjString *name = copyString(vm, "JSON", 4);
    push(vm, OBJ_VAL(name));
    ObjClassNative *klass = newClassNative(vm, name);
    push(vm, OBJ_VAL(klass));

    /**
     * Define Json methods
     */
    defineNativeMethod(vm, klass, "parse", parse);
    defineNativeMethod(vm, klass, "stringify", stringify);

    tableSet(vm, &vm->globals, name, OBJ_VAL(klass));
    pop(vm);
    pop(vm);
}