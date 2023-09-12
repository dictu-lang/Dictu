#define DICTU_ARGPARSE_SOURCE "import System;\n" \
"\n" \
"class Args {\n" \
"    init(private name, private desc) {}\n" \
"}\n" \
"\n" \
"class Parser {\n" \
"    private args;\n" \
"    private preArgs;\n" \
"    private required;\n" \
"\n" \
"    init(private name, private desc, private userUsage = '') {\n" \
"        this.args = Args(name, desc);\n" \
"        this.preArgs = [];\n" \
"        this.required = [];\n" \
"    }\n" \
"\n" \
"    private flagExists(flag) {\n" \
"        for (var i = 0; i < this.preArgs.len(); i+=1) {\n" \
"            if (this.preArgs[i]['flag'] == flag) {\n" \
"                return true;\n" \
"            }\n" \
"        }\n" \
"\n" \
"        return false;\n" \
"    }\n" \
"\n" \
"    private addFlag(flagType, flag, desc, required, ...metavar) {\n" \
"        if (not this.flagExists(flag)) {\n" \
"            var arg = {\n" \
"                'type': flagType,\n" \
"                'flag': flag,\n" \
"                'desc': desc,\n" \
"                'required': required\n" \
"            };\n" \
"\n" \
"            if (metavar.len() == 1) {\n" \
"                arg['metavar'] = metavar[0];\n" \
"            }\n" \
"\n" \
"            this.preArgs.push(arg);\n" \
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
"\n" \
"            for (var i = 0; i < this.preArgs.len(); i+=1) {\n" \
"                u += '    {}    {}{}\n'.format(\n" \
"                    this.preArgs[i]['flag'],\n" \
"                    this.preArgs[i]['desc'],\n" \
"                    this.preArgs[i]['required'] ? '    Required' : ''\n" \
"                );\n" \
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
"                    found += 1;\n" \
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
"    private fillEmpty() {\n" \
"        for (var i = 0; i < this.preArgs.len(); i += 1) {\n" \
"            const arg = this.preArgs[i];\n" \
"\n" \
"            if (arg.get('metavar') and not this.args.getAttribute(arg['metavar'])) {\n" \
"                this.args.setAttribute(arg['metavar'], nil);\n" \
"\n" \
"                continue;\n" \
"            }\n" \
"\n" \
"            const flag = arg['flag'].replace('-', '');\n" \
"            if (not this.args.getAttribute(flag)) {\n" \
"                this.args.setAttribute(flag, nil);\n" \
"            }\n" \
"        }\n" \
"    }\n" \
"\n" \
"    parse() {\n" \
"        for (var i = 0; i < System.argv.len(); i+=1) {\n" \
"            for (var j = 0; j < this.preArgs.len(); j+=1) {\n" \
"                if (System.argv[i] == this.preArgs[j]['flag']) {\n" \
"                    if (this.preArgs[j]['type'] == 'bool') {\n" \
"                        if (this.preArgs[j].exists('metavar') and this.preArgs[j]['metavar'] != '') {\n" \
"                            this.args.setAttribute(this.preArgs[j]['metavar'], true);\n" \
"                        } else {\n" \
"                            this.args.setAttribute(this.preArgs[j]['flag'].replace('-', ''), true);\n" \
"                        }\n" \
"                        \n" \
"                    } else if (this.preArgs[j]['type'] == 'list') {\n" \
"                        if (i == (System.argv.len() - 1) or System.argv[i+1][0] == '-') {\n" \
"                            return Error('{} requires an argument'.format(System.argv[i]));\n" \
"                        }\n" \
"\n" \
"                        if (this.preArgs[j].exists('metavar') and this.preArgs[j]['metavar'] != '') {\n" \
"                            this.args.setAttribute(this.preArgs[j]['metavar'], System.argv[i+1].split(','));   \n" \
"                        } else {\n" \
"                            this.args.setAttribute(this.preArgs[j]['flag'].replace('-', ''), System.argv[i+1].split(','));\n" \
"                        }\n" \
"                    } else if (this.preArgs[j]['type'] == 'number') {\n" \
"                        if (i == (System.argv.len() - 1)) {\n" \
"                            return Error('{} requires an argument'.format(System.argv[i]));\n" \
"                        }\n" \
"\n" \
"                        const res = System.argv[i+1].toNumber();\n" \
"                        if (not res.success()) {\n" \
"                            return Error('{} arg must be a number'.format(System.argv[i]));\n" \
"                        }\n" \
"\n" \
"                        if (this.preArgs[j].exists('metavar') and this.preArgs[j]['metavar'] != '') {\n" \
"                            print(this.preArgs[j]['metavar']);\n" \
"                            this.args.setAttribute(this.preArgs[j]['metavar'], res.unwrap());\n" \
"                        } else {\n" \
"                            this.args.setAttribute(this.preArgs[j]['flag'].replace('-', ''), res.unwrap());\n" \
"                        }\n" \
"                    } else {\n" \
"                        if (i == (System.argv.len() - 1) or System.argv[i+1][0] == '-') {\n" \
"                            return Error('{} requires an argument'.format(System.argv[i]));\n" \
"                        }\n" \
"\n" \
"                        if (this.preArgs[j].exists('metavar') and this.preArgs[j]['metavar'] != '') {\n" \
"                            this.args.setAttribute(this.preArgs[j]['metavar'], System.argv[i+1]);\n" \
"                        } else {\n" \
"                            this.args.setAttribute(this.preArgs[j]['flag'].replace('-', ''), System.argv[i+1]);\n" \
"                        }\n" \
"                    }\n" \
"                }\n" \
"            }\n" \
"        }\n" \
"\n" \
"        if (not this.hasRequired()) {\n" \
"            return Error('1 or more required flags missing');\n" \
"        }\n" \
"\n" \
"        this.fillEmpty();\n" \
"\n" \
"        return Success(this.args);\n" \
"    }\n" \
"}\n" \

