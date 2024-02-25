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

typedef struct {
    long time;    /* timestamp */
	bool is_local; 
} Datetime;


typedef struct {
    unsigned long seconds;
} Duration;

Value createDatetimeModule(DictuVM *vm);

#endif //dictu_datetime_h
