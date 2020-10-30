#ifndef dictu_datetime_h
#define dictu_datetime_h

#define _XOPEN_SOURCE
#define __USE_XOPEN

#include <time.h>

#include "optionals.h"

ObjModule *createDatetimeModule(VM *vm);

#endif //dictu_datetime_h
