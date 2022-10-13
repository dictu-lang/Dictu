#ifndef dictu_env_h
#define dictu_env_h

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../optionals.h"
#include "../../vm/vm.h"

Value createEnvModule(DictuVM *vm);

#endif //dictu_env_h
