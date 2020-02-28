#ifndef dictu_copy_h
#define dictu_copy_h

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../vm.h"
#include "../memory.h"

ObjList *copyList(ObjList *oldList, bool shallow);

ObjDict *copyDict(ObjDict *oldDict, bool shallow);

ObjInstance *copyInstance(ObjInstance *oldInstance, bool shallow);

uint32_t hash(char *str);

#endif //dictu_copy_h
