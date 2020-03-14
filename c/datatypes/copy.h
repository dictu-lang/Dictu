#ifndef dictu_copy_h
#define dictu_copy_h

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../vm.h"
#include "../memory.h"

ObjList *copyList(VM *vm, ObjList *oldList, bool shallow);

ObjDict *copyDict(VM *vm, ObjDict *oldDict, bool shallow);

ObjInstance *copyInstance(VM *vm, ObjInstance *oldInstance, bool shallow);

uint32_t hash(char *str);

#endif //dictu_copy_h
