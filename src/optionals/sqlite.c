#include "sqlite.h"

typedef struct {
    sqlite3 *db;
    bool open;
} Database;

typedef struct {
    sqlite3 *db;
    sqlite3_stmt *stmt;
} Result;

#define AS_SQLITE_DATABASE(v) ((Database*)AS_ABSTRACT(v)->data)

ObjAbstract *newSqlite(DictuVM *vm);

static int countParameters(char *query) {
    int length = strlen(query);
    int count = 0;

    for (int i = 0; i < length; ++i) {
        if (query[i] == '?') count++;
    }

    return count;
}

void bindValue(sqlite3_stmt *stmt, int index, Value value) {
    if (IS_NUMBER(value)) {
        sqlite3_bind_double(stmt, index, AS_NUMBER(value));
        return;
    }

    if (IS_NIL(value)) {
        sqlite3_bind_null(stmt, index);
        return;
    }

    if (IS_STRING(value)) {
        ObjString *string = AS_STRING(value);
        sqlite3_bind_text(stmt, index, string->chars, string->length, SQLITE_TRANSIENT);
    }
}

static Value execute(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1 && argCount != 2) {
        runtimeError(vm, "execute() takes 1 or 2 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[1])) {
        runtimeError(vm, "execute() first argument must be a string.");
        return EMPTY_VAL;
    }

    Database *db = AS_SQLITE_DATABASE(args[0]);

    if (!db->open) {
        return newResultError(vm, "Database connection is closed");
    }

    char *sql = AS_CSTRING(args[1]);
    ObjList *list = NULL;
    int parameterCount = countParameters(sql);
    int argumentCount = 0;

    if (argCount == 2) {
        if (!IS_LIST(args[2])) {
            runtimeError(vm, "execute() second argument must be a list.");
            return EMPTY_VAL;
        }

        list = AS_LIST(args[2]);
        argumentCount = list->values.count;
    }

    if (parameterCount != argumentCount) {
        runtimeError(vm, "execute() has %d parameters but %d were given", parameterCount, argumentCount);
        return EMPTY_VAL;
    }

    Result result;

    int err = sqlite3_prepare_v2(db->db, sql, -1, &result.stmt, NULL);
    if (err != SQLITE_OK) {
        char *error = (char *)sqlite3_errmsg(db->db);
        return newResultError(vm, error);
    }

    if (parameterCount != 0 && list != NULL) {
        for (int i = 0; i < parameterCount; ++i) {
            bindValue(result.stmt, i + 1, list->values.values[i]);
        }
    }

    ObjList *finalList = newList(vm);
    push(vm, OBJ_VAL(finalList));
    bool returnValue = false;

    for (;;) {
        err = sqlite3_step(result.stmt);
        if (err != SQLITE_ROW) {
            if (err == SQLITE_DONE) {
                if (sql[0] == 'S' || sql[0] == 's') {
                    // If a select statement returns no results SQLITE_ROW is not used.
                    returnValue = true;
                }

                break;
            }

            sqlite3_finalize(result.stmt);
            char *error = (char *)sqlite3_errmsg(db->db);
            pop(vm);
            return newResultError(vm, error);
        }

        returnValue = true;

        ObjList *rowList = newList(vm);
        push(vm, OBJ_VAL(rowList));

        for (int i = 0; i < sqlite3_column_count(result.stmt); i++) {
            switch (sqlite3_column_type(result.stmt, i)) {
                case SQLITE_NULL: {
                    writeValueArray(vm, &rowList->values, NIL_VAL);
                    break;
                }

                case SQLITE_INTEGER:
                case SQLITE_FLOAT: {
                    writeValueArray(vm, &rowList->values, NUMBER_VAL(sqlite3_column_double(result.stmt, i)));
                    break;
                }

                case SQLITE_TEXT: {
                    char *s = (char *)sqlite3_column_text(result.stmt, i);
                    ObjString *string = copyString(vm, s, strlen(s));
                    push(vm, OBJ_VAL(string));
                    writeValueArray(vm, &rowList->values, OBJ_VAL(string));
                    pop(vm);
                    break;
                }
            }
        }

        writeValueArray(vm, &finalList->values, OBJ_VAL(rowList));
        pop(vm);
    }

    sqlite3_finalize(result.stmt);
    pop(vm);

    if (returnValue) {
        return newResultSuccess(vm, OBJ_VAL(finalList));
    }

    return newResultSuccess(vm, NIL_VAL);
}

static Value closeConnection(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "close() takes no arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    Database *db = AS_SQLITE_DATABASE(args[0]);

    if (db->open) {
        sqlite3_close(db->db);
        db->open = false;
    }

    return NIL_VAL;
}

static Value connectSqlite(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "connect() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "connect() argument must be a string");
        return EMPTY_VAL;
    }

    ObjAbstract *abstract = newSqlite(vm);
    Database *db = abstract->data;
    char *name = AS_CSTRING(args[0]);

    /* Open database */
    int err = sqlite3_open(name, &db->db);

    if (err) {
        char *error = (char *)sqlite3_errmsg(db->db);
        return newResultError(vm, error);
    }

    sqlite3_stmt *res;
    err = sqlite3_prepare_v2(db->db, "PRAGMA foreign_keys = ON;", -1, &res, 0);

    if (err) {
        char *error = (char *)sqlite3_errmsg(db->db);
        return newResultError(vm, error);
    }

    sqlite3_finalize(res);

    return newResultSuccess(vm, OBJ_VAL(abstract));
}

void freeSqlite(DictuVM *vm, ObjAbstract *abstract) {
    Database *db = (Database*)abstract->data;
    if (db->open) {
        sqlite3_close(db->db);
        db->open = false;
    }
    FREE(vm, Database, abstract->data);
}

char *sqliteToString(ObjAbstract *abstract) {
    UNUSED(abstract);

    char *sqliteString = malloc(sizeof(char) * 9);
    snprintf(sqliteString, 9, "<Sqlite>");
    return sqliteString;
}

ObjAbstract *newSqlite(DictuVM *vm) {
    ObjAbstract *abstract = newAbstract(vm, freeSqlite, sqliteToString);
    push(vm, OBJ_VAL(abstract));

    Database *db = ALLOCATE(vm, Database, 1);
    db->open = true;

    /**
     * Setup Sqlite object methods
     */
    defineNative(vm, &abstract->values, "execute", execute);
    defineNative(vm, &abstract->values, "close", closeConnection);

    abstract->data = db;
    pop(vm);

    return abstract;
}

Value createSqliteModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Sqlite", 6);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Sqlite methods
     */
    defineNative(vm, &module->values, "connect", connectSqlite);

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}