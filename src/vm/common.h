#ifndef dictu_common_h
#define dictu_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define UNUSED(__x__) (void) __x__

#define NAN_TAGGING
#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION
#define DEBUG_TRACE_GC
#define DEBUG_TRACE_MEM

#ifndef _MSC_VER
#define COMPUTED_GOTO
#endif

#undef DEBUG_PRINT_CODE
#undef DEBUG_TRACE_EXECUTION
#undef DEBUG_TRACE_GC
#undef DEBUG_TRACE_MEM

// #define DEBUG_STRESS_GC
// #define DEBUG_FINAL_MEM

#define UINT8_COUNT (UINT8_MAX + 1)

#endif
