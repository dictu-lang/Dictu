#ifndef dictu_socket_h
#define dictu_socket_h

#include "optionals.h"
#include "../vm/vm.h"
#include "../vm/memory.h"
#include "../vm/object.h"

#ifdef __FreeBSD__
#include <netinet/in.h>
#endif

ObjModule *createSocketModule(VM *vm);

#endif //dictu_socket_h
