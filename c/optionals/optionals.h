#ifndef dictu_optionals_h
#define dictu_optionals_h

#include "../util.h"
#include "math.h"
#include "env.h"
#include "system.h"
#include "json.h"
#include "http.h"
#include "path.h"
#include "c.h"
#include "datetime.h"
#include "socket.h"

#define GET_SELF_CLASS \
  AS_MODULE(args[-1])

#define SET_ERRNO(module_)                                              \
  defineNativeProperty(vm, &module_->values, "errno", NUMBER_VAL(errno))

#define GET_ERRNO(module_)({                         \
  Value errno_value = 0;                             \
  ObjString *name = copyString(vm, "errno", 5);      \
  tableGet(&module_->values, name, &errno_value);    \
  errno_value;                                       \
})

typedef ObjModule *(*BuiltinModule)(VM *vm);

typedef struct {
    char *name;
    BuiltinModule module;
} BuiltinModules;

ObjModule *importBuiltinModule(VM *vm, int index);

int findBuiltinModule(char *name, int length);

#endif //dictu_optionals_h
