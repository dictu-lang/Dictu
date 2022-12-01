#define DICTU_ENV_SOURCE "import Argparse; \
import System; \
\
class Arg { \
    init(argType, flag, desc, required) { \
        this.setAttribute('type', argType); \
        this.setAttribute('flag', flag); \
        this.setAttribute('desc', desc); \
        this.setAttribute('required', required); \
    } \
} \
\
class Args { \
    init() {} \
} \
\
class Parser { \
    private name; \
    private desc; \
    private usage; \
\
    var args = []; \
\
    init(var name, var desc, var usage) {} \
\
    addString(flag, desc, required, ...metavar) { \
        this.args.push(Arg('string', flag, desc, required)); \
    } \
\
    addNumber(flag, desc, required, ...metavar) { \
        this.args.push(Arg('number', flag, desc, required)); \
    } \
\
    addBool(flag, desc, required, ...metavar) { \
        this.args.push(Arg('bool', flag, desc, required)); \
    } \
\
    addList(flag, desc, required, ...metavar) { \
        this.args.push(Arg('list', flag, desc, required)); \
    } \
\
    usage() { \
        if (this.usage == '') { \
            return ""; \
        } \
\
        return this.usage; \
    } \
\
    parse() { \
        var args = Args(); \
\
        for (var i = 0; i < System.argv.len(); i+=1) { \
            for (var j = 0; j < this.args.len(); j+=1) { \
                if (System.argv[i] == this.args[j].flag) { \
                    if (this.args[j].type == 'bool') { \
                        args.setAttribute(this.args[j].flag.replace('-', ''), true); \
                    } else if (this.args[j].type == 'list') { \
                        if (i == (System.argv.len() - 1) or System.argv[i+1][0] == '-') { \
                            return Error('{} requires an argument'); \
                        } \
\
                        args.setAttribute(this.args[j].flag.replace('-', ''), System.argv[i+1].split(',')); \
                    } else if (this.args[j].type == 'number') { \
                        const res = System.argv[i+1].toNumber(); \
                        if (not res.success()) { \
                            return Error('{} arg must be a number'.format(System.argv[i])); \
                        } \
\
                        args.setAttribute(this.args[j].flag.replace('-', ''), res.unwrap()); \
                    } else { \
                        if (i == (System.argv.len() - 1) or System.argv[i+1][0] == '-') { \
                            return Error('{} requires an argument'); \
                        } \
\
                        args.setAttribute(this.args[j].flag.replace('-', ''), System.argv[i+1]); \
                    } \
                } \
            } \
        } \
\
        return Success(args); \
    } \
} \
"
