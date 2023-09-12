#define DICTU_OBJECT_SOURCE "import Object;\n" \
"\n" \
"def createFrom(className, ...arguments) {\n" \
"    return Object.__getClassRef(className).matchWrap(\n" \
"        def (klass) => klass(...arguments),\n" \
"        def (error) => error\n" \
"    );\n" \
"}\n" \

