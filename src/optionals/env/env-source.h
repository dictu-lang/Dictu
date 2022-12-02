#define DICTU_ENV_SOURCE "import Env;\n" \
"\n" \
"def readFile(path='.env') {\n" \
"    const SPLIT_DELIMITER = '=';\n" \
"    const COMMENT = '#';\n" \
"\n" \
"    with(path, 'r') {\n" \
"        var line;\n" \
"        var lineCount = 0;\n" \
"        // When you reach the end of the file, nil is returned\n" \
"        while((line = file.readLine()) != nil) {\n" \
"            lineCount = lineCount + 1;\n" \
"            if (not line or line.startsWith(COMMENT))\n" \
"                continue;\n" \
"\n" \
"            if (not line.contains('='))\n" \
"                return Error('Malformed entry on line {}'.format(lineCount));\n" \
"\n" \
"            const [variable, rawValue] = line.split(SPLIT_DELIMITER, 1);\n" \
"            // Strip out any in-line comments\n" \
"            const value = rawValue.split(COMMENT, 1);\n" \
"            const result = Env.set(variable.strip(), value[0].strip());\n" \
"\n" \
"            if (not result.success()) {\n" \
"                return result;\n" \
"            }\n" \
"        }\n" \
"    }\n" \
"\n" \
"    return Success(nil);\n" \
"}\n" \