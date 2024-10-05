#define DICTU_DICT_SOURCE "/**\n" \
" * This file contains all the methods for Dicts written in Dictu that\n" \
" * are unable to be written in C due to re-enterability issues.\n" \
" *\n" \
" * We should always strive to write methods in C where possible.\n" \
" */\n" \
"def forEach(dict, func) {\n" \
"    const dictKeys = dict.keys();\n" \
"\n" \
"    for (var i = 0; i < dictKeys.len(); i += 1) {\n" \
"        func(dictKeys[i], dict[dictKeys[i]]);\n" \
"    }\n" \
"}\n" \
"\n" \
"def merge(dict, anotherDict) {\n" \
"    const newDict = dict.copy();\n" \
"\n" \
"    forEach(anotherDict, def (key, value) => {\n" \
"       newDict[key] = value;\n" \
"    });\n" \
"\n" \
"    return newDict;\n" \
"}\n" \
"\n" \
"def toObj(dict, obj) {\n" \
"    dict.forEach(def(k, v) => {\n" \
"        if (type(k) != 'string') {\n" \
"            const fieldName = k.toString();\n" \
"            obj.setAttribute(fieldName, v);\n" \
"            return;\n" \
"        }\n" \
"        obj.setAttribute(k, v);\n" \
"    });\n" \
"\n" \
"    return obj;\n" \
"}\n" \

