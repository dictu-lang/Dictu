#define DICTU_HTTP_SOURCE "class Response {\n" \
"    // content, headers and statusCode set via C\n" \
"    init() {}\n" \
"\n" \
"    json() {\n" \
"        // Import needs to be local to ensure HTTP is defined correctly\n" \
"        import JSON;\n" \
"\n" \
"        return JSON.parse(this.content);\n" \
"    }\n" \
"}\n" \
"\n" \

