#ifndef dictu_uuid_h
#define dictu_uuid_h

#include "optionals.h"
#include "../vm/vm.h"
#include "../vm/memory.h"

#include <uuid/uuid.h>

Value createUuidModule(DictuVM *vm);

#endif //dictu_uuid_h
