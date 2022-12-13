#define DICTU_ARGPARSE_SOURCE "import Argparse;\n" \
"import System;\n" \
"\n" \
"class Args {\n" \
"    init(private name, private desc) {}\n" \
"}\n" \
"\n" \
"class Parser {\n" \
"    private name;\n" \
"    private desc;\n" \
"    private userUsage;\n" \
"    private args;\n" \
"\n" \
"    var preArgs = [];\n" \
"    var required = [];\n" \
"\n" \
"    init(var name, var desc, var userUsage) {\n" \
"        this.args = Args(name, desc);\n" \
"    }\n" \
"\n" \
"    private flagExists(flag) {\n" \
"        for (var i = 0; i < this.preArgs.len(); i+=1) {\n" \
"            if (this.preArgs[i]['flag'] == flag) {\n" \
"                return true;\\n" \
"            }\n" \
"        }\n" \
"\n" \
"        return false;\n" \
"    }\n" \
"\n" \
"    private addFlag(flagType, flag, desc, required, ...metavar) {\n" \
"        if (not this.flagExists(flag)) {\n" \
"            this.preArgs.push({\n" \
"                'type': flagType,\n" \
"                'flag': flag,\n" \
"                'desc': desc,\n" \
"                'required': required,\n" \
"                'metavr': metavar\n" \
"            });\n" \
"\n" \
"            if (required) {\n" \
"                this.required.push(flag);\n" \
"            }\n" \
"        }\n" \
"    }\n" \
"\n" \
"    addString(flag, desc, required, ...metavar) {\n" \
"        this.addFlag('string', flag, desc, required, ...metavar);\n" \
"    }\n" \
"\n" \
"    addNumber(flag, desc, required, ...metavar) {\n" \
"        this.addFlag('number', flag, desc, required, ...metavar);\n" \
"    }\n" \
"\n" \
"    addBool(flag, desc, required, ...metavar) {\n" \
"        this.addFlag('bool', flag, desc, required, ...metavar);\n" \
"    }\n" \
"\n" \
"    addList(flag, desc, required, ...metavar) {\n" \
"        this.addFlag('list', flag, desc, required, ...metavar);\n" \
"    }\n" \
"\n" \
"    usage() {\n" \
"        if (this.userUsage == '') {\n" \
"            var u = 'usage: {}\n    {}\n\n'.format(this.name, this.desc);\n" \
"            for (var i = 0; i < this.preArgs.len(); i+=1) {\n" \
"                u += '    {}    {}\n'.format(this.preArgs[i]['flag'], this.preArgs[i]['desc']);\n" \
"            }\n" \
"\n" \
"            return u;\n" \
"        }\n" \
"\n" \
"        return this.userUsage;\n" \
"    }\n" \
"\n" \
"    private hasRequired() {\n" \
"        var found = 0;\n" \
"        for (var i = 0; i < System.argv.len(); i+=1) {\n" \
"            for (var j = 0; j < this.required.len(); j+=1) {\n" \
"                if (System.argv[i] == this.required[j]) {\n" \
"                    found += 1;\\n" \
"                }\n" \
"            }\n" \
"        }\n" \
"\n" \
"        if (found == this.required.len()) {\n" \
"            return true;\n" \
"        }\n" \
"\n" \
"        return false;\n" \
"    }\n" \
"\n" \
"    parse() {\n" \
"        for (var i = 0; i < System.argv.len(); i+=1) {\n" \
"            for (var j = 0; j < this.preArgs.len(); j+=1) {\n" \
"                if (System.argv[i] == this.preArgs[j]['flag']) {\n" \
"                    if (this.preArgs[j]['type'] == 'bool') {\n" \
"                        this.args.setAttribute(this.preArgs[j]['flag'].replace('-', ''), true);\n" \
"                    } else if (this.preArgs[j]['type'] == 'list') {\n" \
"                        if (i == (System.argv.len() - 1) or System.argv[i+1][0] == '-') {\n" \
"                            return Error('{} requires an argument');\n" \
"                        }\n" \
"\n" \
"                        this.args.setAttribute(this.preArgs[j]['flag'].replace('-', ''), System.argv[i+1].split(','));\n" \
"                    } else if (this.preArgs[j]['type'] == 'number') {\n" \
"                        if (i == (System.argv.len() - 1) or System.argv[i+1][0] == '-') {\n" \
"                            return Error('{} requires an argument');\n" \
"                        }\n" \
"                        const res = System.argv[i+1].toNumber();\n" \
"                        if (not res.success()) {\n" \
"                            return Error('{} arg must be a number'.format(System.argv[i]));\n" \
"                        }\n" \
"\n" \
"                        this.args.setAttribute(this.preArgs[j]['flag'].replace('-', ''), res.unwrap());\n" \
"                    } else {\n" \
"                        if (i == (System.argv.len() - 1) or System.argv[i+1][0] == '-') {\n" \
"                            return Error('{} requires an argument');\n" \
"                        }\n" \
"\n" \
"                        this.args.setAttribute(this.preArgs[j]['flag'].replace('-', ''), System.argv[i+1]);\n" \
"                    }\n" \
"                }\n" \
"            }\n" \
"        }\n" \
"\n" \
"        if (not this.hasRequired()) {\n" \
"            return Error('1 or more required flags missing');\n" \
"        }\n" \
"\n" \
"        return Success(this.args);\n" \
"    }\n" \
"}\n" \

