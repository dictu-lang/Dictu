#ifndef dictu_json_h
#define dictu_json_h

#include "json/jsonParseLib.h"
#include "json/jsonBuilderLib.h"
#include "optionals.h"
#include "../vm/vm.h"

Value createJSONModule(DictuVM *vm);
Value stringify(DictuVM *vm, int argCount, Value *args);

#endif //dictu_json_h
