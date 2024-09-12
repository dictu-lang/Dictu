#include "c.h"

#ifdef _WIN32
#define strerror_r(ERRNO, BUF, LEN) strerror_s(BUF, LEN, ERRNO)
#endif

void getStrerror(char *buf, int error) {
#ifdef POSIX_STRERROR
    int intval = strerror_r(error, buf, MAX_ERROR_LEN);

    if (intval == 0) {
        return;
    }

    /* it is safe to assume that we do not reach here */
    memcpy(buf, "Buffer is too small", 19);
    buf[19] = '\0';
#else
    strerror_r(error, buf, MAX_ERROR_LEN);
#endif
}
