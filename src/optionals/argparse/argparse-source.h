#define DICTU_ENV_SOURCE "import Argparse; \
import System; \
\
class Args { \
    init(private name, private desc) {} \
\
    toString() { \
        return 'usage: {}\n \
    {}\n \
        '.format(this.name, this.desc); \
    } \
} \
\
class Parser { \
    private name; \
    private desc; \
    private userUsage; \
    private args; \
\
    var preArgs = []; \
    var required = []; \
\
    init(var name, var desc, var userUsage) { \
        this.args = Args(name, desc); \
    } \
\
    private flagExists(flag) { \
        for (var i = 0; i < this.preArgs.len(); i+=1) { \
            if (this.preArgs[i]['flag'] == flag) { \
                return true;\
            } \
        } \
\
        return false; \
    } \
\
    private addFlag(flagType, flag, desc, required, ...metavar) { \
        if (not this.flagExists(flag)) { \
            this.preArgs.push({ \
                'type': flagType, \
                'flag': flag, \
                'desc': desc, \
                'required': required, \
                'metavr': metavar \
            }); \
\
            if (required) { \
                this.required.push(flag); \
            } \
        } \
    } \
\
    addString(flag, desc, required, ...metavar) { \
        this.addFlag('string', flag, desc, required, ...metavar); \
    } \
\
    addNumber(flag, desc, required, ...metavar) { \
        this.addFlag('number', flag, desc, required, ...metavar); \
    } \
\
    addBool(flag, desc, required, ...metavar) { \
        this.addFlag('bool', flag, desc, required, ...metavar); \
    } \
\
    addList(flag, desc, required, ...metavar) { \
        this.addFlag('list', flag, desc, required, ...metavar); \
    } \
\
    usage() { \
        if (this.userUsage == '') { \
            var u = 'usage: {}\n    {}\n\n'.format(this.name, this.desc); \
            for (var i = 0; i < this.preArgs.len(); i+=1) { \
                u += '    {}    {}\n'.format(this.preArgs[i]['flag'], this.preArgs[i]['desc']); \
            } \
\
            return u; \
        } \
\
        return this.userUsage; \
    } \
\
    private hasRequired() { \
        var found = 0; \
        for (var i = 0; i < System.argv.len(); i+=1) { \
            for (var j = 0; j < this.required.len(); j+=1) { \
                if (System.argv[i] == this.required[j]) { \
                    found += 1;\
                } \
            } \
        } \
\
        if (found == this.required.len()) { \
            return true; \
        } \
\
        return false; \
    } \
\
    parse() { \
        for (var i = 0; i < System.argv.len(); i+=1) { \
            for (var j = 0; j < this.preArgs.len(); j+=1) { \
                if (System.argv[i] == this.preArgs[j]['flag']) { \
                    if (this.preArgs[j]['type'] == 'bool') { \
                        this.args.setAttribute(this.preArgs[j]['flag'].replace('-', ''), true); \
                    } else if (this.preArgs[j]['type'] == 'list') { \
                        if (i == (System.argv.len() - 1) or System.argv[i+1][0] == '-') { \
                            return Error('{} requires an argument'); \
                        } \
\
                        this.args.setAttribute(this.preArgs[j]['flag'].replace('-', ''), System.argv[i+1].split(',')); \
                    } else if (this.preArgs[j]['type'] == 'number') { \
                        if (i == (System.argv.len() - 1) or System.argv[i+1][0] == '-') { \
                            return Error('{} requires an argument'); \
                        } \
                        const res = System.argv[i+1].toNumber(); \
                        if (not res.success()) { \
                            return Error('{} arg must be a number'.format(System.argv[i])); \
                        } \
\
                        this.args.setAttribute(this.preArgs[j]['flag'].replace('-', ''), res.unwrap()); \
                    } else { \
                        if (i == (System.argv.len() - 1) or System.argv[i+1][0] == '-') { \
                            return Error('{} requires an argument'); \
                        } \
\
                        this.args.setAttribute(this.preArgs[j]['flag'].replace('-', ''), System.argv[i+1]); \
                    } \
                } \
            } \
        } \
\
        if (not this.hasRequired()) { \
            return Error('1 or more required flags missing'); \
        } \
\
        return Success(this.args); \
    } \
} \
"
