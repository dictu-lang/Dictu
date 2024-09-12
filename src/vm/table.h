#ifndef dictu_table_h
#define dictu_table_h

#include <stdio.h>

#include "common.h"
#include "value.h"

typedef struct {
    ObjString *key;
    Value value;
    uint32_t psl;
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry *entries;
} Table;

void initTable(Table *table);

void freeTable(DictuVM *vm, Table *table);

bool tableGet(Table *table, ObjString *key, Value *value);

bool tableSet(DictuVM *vm, Table *table, ObjString *key, Value value);

bool tableDelete(DictuVM *vm, Table *table, ObjString *key);

void tableAddAll(DictuVM *vm, Table *from, Table *to);

ObjString *tableFindString(Table *table, const char *chars, int length,
                           uint32_t hash);

void tableRemoveWhite(DictuVM *vm, Table *table);

void grayTable(DictuVM *vm, Table *table);

#endif
