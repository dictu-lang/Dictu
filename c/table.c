#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

void initTable(Table *table) {
    table->count = 0;
    table->capacityMask = -1;
    table->entries = NULL;
}

void freeTable(Table *table) {
    FREE_ARRAY(Entry, table->entries, table->capacityMask + 1);
    initTable(table);
}

static Entry *findEntry(Entry *entries, int capacityMask,
                        ObjString *key) {
    uint32_t index = key->hash & capacityMask;
    Entry *tombstone = NULL;

    for (;;) {
        Entry *entry = &entries[index];

        if (entry->key == NULL) {
            if (IS_NIL(entry->value)) {
                // Empty entry.
                return tombstone != NULL ? tombstone : entry;
            } else {
                // We found a tombstone.
                if (tombstone == NULL) tombstone = entry;
            }
        } else if (entry->key == key) {
            // We found the key.
            return entry;
        }

        index = (index + 1) & capacityMask;
    }
}

bool tableGet(Table *table, ObjString *key, Value *value) {
    if (table->entries == NULL) return false;

    Entry *entry = findEntry(table->entries, table->capacityMask, key);
    if (entry->key == NULL) return false;

    *value = entry->value;
    return true;
}

static void adjustCapacity(Table *table, int capacityMask) {
    Entry *entries = ALLOCATE(Entry, capacityMask + 1);
    for (int i = 0; i <= capacityMask; i++) {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }

    table->count = 0;

    for (int i = 0; i <= table->capacityMask; i++) {
        Entry *entry = &table->entries[i];
        if (entry->key == NULL) continue;

        Entry *dest = findEntry(entries, capacityMask, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    FREE_ARRAY(Entry, table->entries, table->capacityMask + 1);
    table->entries = entries;
    table->capacityMask = capacityMask;
}

bool tableSet(Table *table, ObjString *key, Value value) {
    if (table->count + 1 > (table->capacityMask + 1) * TABLE_MAX_LOAD) {
        // Figure out the new table size.
        int capacityMask = GROW_CAPACITY(table->capacityMask + 1) - 1;
        adjustCapacity(table, capacityMask);
    }

    Entry *entry = findEntry(table->entries, table->capacityMask, key);
    bool isNewKey = entry->key == NULL;
    entry->key = key;
    entry->value = value;

    if (isNewKey) table->count++;
    return isNewKey;
}

bool tableDelete(Table *table, ObjString *key) {
    if (table->count == 0) return false;

    Entry *entry = findEntry(table->entries, table->capacityMask, key);
    if (entry->key == NULL) return false;

    // Place a tombstone in the entry.
    entry->key = NULL;
    entry->value = BOOL_VAL(true);

    return true;
}

void tableAddAll(Table *from, Table *to) {
    for (int i = 0; i <= from->capacityMask; i++) {
        Entry *entry = &from->entries[i];
        if (entry->key != NULL) {
            tableSet(to, entry->key, entry->value);
        }
    }
}

ObjString *tableFindString(Table *table, const char *chars, int length,
                           uint32_t hash) {
    // If the table is empty, we definitely won't find it.
    if (table->entries == NULL) return NULL;

    // Figure out where to insert it in the table. Use open addressing and
    // basic linear probing.

    uint32_t index = hash & table->capacityMask;

    for (;;) {
        Entry *entry = &table->entries[index];

        if (entry->key == NULL) return NULL;
        if (entry->key->length == length &&
            memcmp(entry->key->chars, chars, length) == 0) {
            // We found it.
            return entry->key;
        }

        // Try the next slot.
        index = (index + 1) & table->capacityMask;
    }
}

void tableRemoveWhite(Table *table) {
    for (int i = 0; i <= table->capacityMask; i++) {
        Entry *entry = &table->entries[i];
        if (entry->key != NULL && !entry->key->obj.isDark) {
            tableDelete(table, entry->key);
        }
    }
}

void grayTable(Table *table) {
    for (int i = 0; i <= table->capacityMask; i++) {
        Entry *entry = &table->entries[i];
        grayObject((Obj *) entry->key);
        grayValue(entry->value);
    }
}
