#ifndef dictu_sqlite_h
#define dictu_sqlite_h

#ifdef INCLUDE_SQLITE_LIB
#include <sqlite3.h>
#else
#include "sqlite/sqlite3.h"
#endif


#include "optionals.h"
#include "../vm/vm.h"

ObjModule *createSqliteModule(DictuVM *vm);

#endif //dictu_sqlite_h
