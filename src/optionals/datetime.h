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
    unsigned char seconds;	/* seconds after the minute [0-60] */
	unsigned char minutes;	/* minutes after the hour [0-59] */
	unsigned char hours;	    /* hours since midnight [0-23] */
	unsigned char day;	    /* day of the month [1-31] */
	unsigned char month;		/* months since January [0-11] */
	unsigned short year;	    /* years since 1900 */
} Datetime;

Value createDatetimeModule(DictuVM *vm);

#endif //dictu_datetime_h
