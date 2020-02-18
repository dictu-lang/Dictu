#ifndef dictu_system_h
#define dictu_system_h

#include <time.h>
#include <unistd.h>
#include <limits.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "optionals.h"
#include "../vm.h"
#include "../memory.h"

void createSystemClass();

#endif //dictu_system_h
