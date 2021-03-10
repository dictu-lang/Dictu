#define DICTU_RESULT_SOURCE "/**\n" \
" * This file contains all the methods for Results written in Dictu that\n" \
" * are unable to be written in C due to re-enterability issues.\n" \
" *\n" \
" * We should always strive to write methods in C where possible.\n" \
" */\n" \
"\n" \
"def match(result, successCallback, errorCallback) {\n" \
"    if (result.success()) {\n" \
"        return successCallback(result.unwrap());\n" \
"    }\n" \
"\n" \
"    return errorCallback(result.unwrapError());\n" \
"}\n" \

