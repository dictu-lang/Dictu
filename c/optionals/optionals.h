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

#define GET_SELF_CLASS \
  AS_CLASS_NATIVE(args[-1])

#define SET_ERRNO(klass_)                                              \
  defineNativeProperty(vm, &klass_->properties, "errno", NUMBER_VAL(errno))

#define GET_ERRNO(klass_)({                          \
  Value errno_value = 0;                             \
  ObjString *name = copyString(vm, "errno", 5);      \
  tableGet(&klass_->properties, name, &errno_value); \
  errno_value;                                       \
})

#endif //dictu_optionals_h
