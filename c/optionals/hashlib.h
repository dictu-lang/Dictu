#ifndef dictu_hashlib_h
#define dictu_hashlib_h

#include "optionals.h"
#include "hashlib/utils.h"
#include "hashlib/sha256.h"
#include "hashlib/hmac.h"

ObjModule *createHashlibModule(VM *vm);

#endif //dictu_hashlib_h
