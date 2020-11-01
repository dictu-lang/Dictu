#ifndef dictu_datetime_h
#define dictu_datetime_h

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif

#include <time.h>

#include "optionals.h"

ObjModule *createDatetimeClass(VM *vm);

#endif //dictu_datetime_h
