#ifndef dictu_copy_h
#define dictu_copy_h

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../vm.h"
#include "../memory.h"

ObjList *copyList(DictuVM *vm, ObjList *oldList, bool shallow);

ObjDict *copyDict(DictuVM *vm, ObjDict *oldDict, bool shallow);

ObjInstance *copyInstance(DictuVM *vm, ObjInstance *oldInstance, bool shallow);

#endif //dictu_copy_h
