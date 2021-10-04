#ifndef dictu_hashlib_h
#define dictu_hashlib_h

#include "optionals.h"
#include "hashlib/utils.h"
#include "hashlib/sha256.h"
#include "hashlib/hmac.h"
#include "hashlib/bcrypt/bcrypt.h"

Value createHashlibModule(DictuVM *vm);

#endif //dictu_hashlib_h
