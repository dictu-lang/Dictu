#ifndef dictu_env_h
#define dictu_env_h

#include <stdlib.h>
#include <string.h>

#include "optionals.h"
#include "../vm/vm.h"

ObjModule *createEnvModule(VM *vm);

#endif //dictu_env_h
