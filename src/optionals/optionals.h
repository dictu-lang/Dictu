#ifndef dictu_optionals_h
#define dictu_optionals_h

#include "../vm/util.h"
#include "c.h"
#include "argparse/argparse.h"
#include "math.h"
#include "env/env.h"
#include "io.h"
#include "system.h"
#include "json.h"
#include "log.h"
#include "http/http.h"
#include "path.h"
#include "datetime.h"
#include "socket.h"
#include "net.h"
#include "random.h"
#include "base64.h"
#include "hashlib.h"
#include "sqlite.h"
#include "process.h"
#include "inspect.h"
#include "term.h"
#include "uuid.h"
#include "queue.h"
#include "stack.h"
#include "bigint.h"
#include "object/object.h"
#include "buffer.h"
#include "unittest/unittest.h"
#include "ffi.h"

typedef Value (*BuiltinModule)(DictuVM *vm);

typedef struct {
    char *name;
    BuiltinModule module;
    bool dictuSource;
} BuiltinModules;

Value importBuiltinModule(DictuVM *vm, int index);

int findBuiltinModule(char *name, int length, bool *dictuSource);

#endif //dictu_optionals_h
