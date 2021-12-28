#ifndef _dictu_log_h
#define _dictu_log_h

#include "../vm/vm.h"

#ifdef _WIN32
#define	STDIN_FILENO	0	/* standard input file descriptor */
#define	STDOUT_FILENO	1	/* standard output file descriptor */
#define	STDERR_FILENO	2	/* standard error file descriptor */
#endif

Value createLogModule(DictuVM *vm);

#endif //_dictu_log_h
