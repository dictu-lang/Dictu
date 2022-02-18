#define DICTU_HTTP_SOURCE "class Response {\n" \
"    // content, headers and statusCode set via C\n" \
"    init() {}\n" \
"\n" \
"    json() {\n" \
"        import JSON;\n" \
"\n" \
"        return JSON.parse(this.content);\n" \
"    }\n" \
"}\n" \
"\n" \

