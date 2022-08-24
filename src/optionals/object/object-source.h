#define DICTU_OBJECT_SOURCE "def createFrom(className, ...arguments) {\n" \
"    import Object;\n" \
"\n" \
"    return Object.__getClassRef(className).matchWrap(\n" \
"        def (klass) => klass(),\n" \
"        def (error) => error\n" \
"    );\n" \
"}\n" \

