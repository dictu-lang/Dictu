#define DICTU_LIST_SOURCE "/**\n" \
" * This file contains all the methods for Lists written in Dictu that\n" \
" * are unable to be written in C due to re-enterability issues.\n" \
" *\n" \
" * We should always strive to write methods in C where possible.\n" \
" */\n" \
"def map(list, func) {\n" \
"    const temp = [];\n" \
"\n" \
"    for (var i = 0; i < list.len(); i += 1) {\n" \
"        temp.push(func(list[i]));\n" \
"    }\n" \
"\n" \
"    return temp;\n" \
"}\n" \
"\n" \
"def filter(list, func=def(x) => x) {\n" \
"    const temp = [];\n" \
"\n" \
"    for (var i = 0; i < list.len(); i += 1) {\n" \
"        const result = func(list[i]);\n" \
"        if (result) {\n" \
"            temp.push(list[i]);\n" \
"        }\n" \
"    }\n" \
"\n" \
"    return temp;\n" \
"}\n" \
"\n" \
"def reduce(list, func, initial=0) {\n" \
"    var accumulator = initial;\n" \
"\n" \
"    for (var i = 0; i < list.len(); i += 1) {\n" \
"        accumulator = func(accumulator, list[i]);\n" \
"    }\n" \
"\n" \
"    return accumulator;\n" \
"}\n" \
"\n" \
"def forEach(list, func) {\n" \
"    for (var i = 0; i < list.len(); i += 1) {\n" \
"        func(list[i]);\n" \
"    }\n" \
"}\n" \
"\n" \
"def find(list, func, start=0, end=list.len()) {\n" \
"    for (var i = start; i < end; i += 1) {\n" \
"        if (func(list[i])) {\n" \
"            return list[i];\n" \
"        }\n" \
"    }\n" \
"}\n" \
"\n" \
"def findIndex(list, func, start=0, end=list.len()) {\n" \
"    for (var i = start; i < end; i += 1) {\n" \
"        if (func(list[i])) {\n" \
"            return i;\n" \
"        }\n" \
"    }\n" \
"}\n" \
"\n" \
"def splice(list, index, count, items) {\n" \
"    if (count == 0) {\n" \
"        return list[:index]+items+list[index:];    \n" \
"    }\n" \
"\n" \
"    return list[:index]+items+list[index+count:];\n" \
"}\n" \

