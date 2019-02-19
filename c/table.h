#ifndef dictu_table_h
#define dictu_table_h

#include <stdio.h>

#include "common.h"
#include "value.h"

typedef struct {
    ObjString *key;
    Value value;
} Entry;

typedef struct {
    int count;
    int capacityMask;
    Entry *entries;
} Table;

void initTable(Table *table);

void freeTable(Table *table);

bool tableGet(Table *table, ObjString *key, Value *value);

bool tableSet(Table *table, ObjString *key, Value value);

bool tableDelete(Table *table, ObjString *key);

void tableAddAll(Table *from, Table *to);

ObjString *tableFindString(Table *table, const char *chars, int length,
                           uint32_t hash);

void tableRemoveWhite(Table *table);

void grayTable(Table *table);

#endif
