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
"def merge(dict, anotherDict) {\n"\
"    const dictKeys = dict.keys();\n"\
"    const newDict= anotherDict.copy();\n"\
"\n"\
"    for (var i = 0; i < dictKeys.len(); i += 1) {\n"\
"       newDict[dictKeys[i]]=dict[dictKeys[i]];\n"\
"    }\n"\
"    return newDict;\n"\
"\n}"\

