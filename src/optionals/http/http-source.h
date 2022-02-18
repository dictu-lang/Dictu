#define DICTU_HTTP_SOURCE "import JSON;\n" \
"\n" \
"class Response {\n" \
"    // content, headers and statusCode set via C\n" \
"    init() {}\n" \
"\n" \
"    json() {\n" \
"        return JSON.parse(this.content);\n" \
"    }\n" \
"}\n" \
"\n" \

