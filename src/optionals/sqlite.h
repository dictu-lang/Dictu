#ifndef dictu_sqlite_h
#define dictu_sqlite_h

#include "sqlite/sqlite3.h"

#include "optionals.h"
#include "../vm/vm.h"

ObjModule *createSqliteModule(DictuVM *vm);

#endif //dictu_sqlite_h
