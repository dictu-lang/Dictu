#ifndef dictu_uuid_h
#define dictu_uuid_h

#include "optionals.h"
#include "../vm/vm.h"
#include "../vm/memory.h"

#ifndef _WIN32
#include <uuid/uuid.h>
#endif 

Value createUuidModule(DictuVM *vm);

#endif //dictu_uuid_h
