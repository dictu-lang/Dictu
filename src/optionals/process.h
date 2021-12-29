#ifndef dictu_process_h
#define dictu_process_h

#ifndef _WIN32
#include <sys/wait.h>
#endif // !_WIN32

#include "optionals.h"

Value createProcessModule(DictuVM *vm);

#endif //dictu_process_h
