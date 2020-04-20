#include <string.h>

#include "memory.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

void initTable(Table *table) {
    table->count = 0;
    table->capacityMask = -1;
    table->entries = NULL;
}

void freeTable(VM *vm, Table *table) {
    FREE_ARRAY(vm, Entry, table->entries, table->capacityMask + 1);
    initTable(table);
}

bool tableGet(Table *table, ObjString *key, Value *value) {
    if (table->count == 0) return false;

    Entry *entry;
    uint32_t index = key->hash & table->capacityMask;
    uint32_t psl = 0;

    for (;;) {
        entry = &table->entries[index];

        if (entry->key == NULL || psl > entry->psl) {
            return false;
        }

        if (entry->key == key) {
            break;
        }

        index = (index + 1) & table->capacityMask;
        psl++;
    }

    *value = entry->value;
    return true;
}

static void adjustCapacity(VM *vm, Table *table, int capacityMask) {
    Entry *entries = ALLOCATE(vm, Entry, capacityMask + 1);
    for (int i = 0; i <= capacityMask; i++) {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
        entries[i].psl = 0;
    }

    Entry *oldEntries = table->entries;
    int oldMask = table->capacityMask;

    table->count = 0;
    table->entries = entries;
    table->capacityMask = capacityMask;

    for (int i = 0; i <= oldMask; i++) {
        Entry *entry = &oldEntries[i];
        if (entry->key == NULL) continue;

        tableSet(vm, table, entry->key, entry->value);
    }

    FREE_ARRAY(vm, Entry, oldEntries, oldMask + 1);
}

bool tableSet(VM *vm, Table *table, ObjString *key, Value value) {
    if (table->count + 1 > (table->capacityMask + 1) * TABLE_MAX_LOAD) {
        // Figure out the new table size.
        int capacityMask = GROW_CAPACITY(table->capacityMask + 1) - 1;
        adjustCapacity(vm, table, capacityMask);
    }

    uint32_t index = key->hash & table->capacityMask;
    Entry *bucket;
    bool isNewKey = false;

    Entry entry;
    entry.key = key;
    entry.value = value;
    entry.psl = 0;

    for (;;) {
        bucket = &table->entries[index];

        if (bucket->key == NULL) {
            isNewKey = true;
            break;
        } else {
            if (bucket->key == key) {
                break;
            }

            if (entry.psl > bucket->psl) {
                isNewKey = true;
                Entry tmp = entry;
                entry = *bucket;
                *bucket = tmp;
            }
        }

        index = (index + 1) & table->capacityMask;
        entry.psl++;
    }

    *bucket = entry;
    if (isNewKey) table->count++;
    return isNewKey;
}

bool tableDelete(VM *vm, Table *table, ObjString *key) {
    if (table->count == 0) return false;

    int capacityMask = table->capacityMask;
    uint32_t index = key->hash & table->capacityMask;
    uint32_t psl = 0;
    Entry *entry;

    for (;;) {
        entry = &table->entries[index];

        if (entry->key == NULL || psl > entry->psl) {
            return false;
        }

        if (entry->key == key) {
            break;
        }

        index = (index + 1) & capacityMask;
        psl++;
    }

    table->count--;

    for (;;) {
        Entry *nextEntry;
        entry->key = NULL;
        entry->value = NIL_VAL;
        entry->psl = 0;

        index = (index + 1) & capacityMask;
        nextEntry = &table->entries[index];

        /*
         * Stop if we reach an empty bucket or hit a key which
         * is in its base (original) location.
         */
        if (nextEntry->key == NULL || nextEntry->psl == 0) {
            break;
        }

        nextEntry->psl--;
        *entry = *nextEntry;
        entry = nextEntry;
    }

    // TODO: Add constant for table load factor
    if (table->count - 1 < table->capacityMask * 0.35) {
        // Figure out the new table size.
        capacityMask = SHRINK_CAPACITY(table->capacityMask);
        adjustCapacity(vm, table, capacityMask);
    }

    return true;
}

void tableAddAll(VM *vm, Table *from, Table *to) {
    for (int i = 0; i <= from->capacityMask; i++) {
        Entry *entry = &from->entries[i];
        if (entry->key != NULL) {
            tableSet(vm, to, entry->key, entry->value);
        }
    }
}

// TODO: Return entry here rather than string
ObjString *tableFindString(Table *table, const char *chars, int length,
                           uint32_t hash) {
    // If the table is empty, we definitely won't find it.
    if (table->count == 0) return NULL;

    // Figure out where to insert it in the table. Use open addressing and
    // basic linear probing.

    uint32_t index = hash & table->capacityMask;
    uint32_t psl = 0;

    for (;;) {
        Entry *entry = &table->entries[index];

        if (entry->key == NULL || psl > entry->psl) {
            return NULL;
        }

        if (entry->key->length == length &&
            entry->key->hash == hash &&
            memcmp(entry->key->chars, chars, length) == 0) {
            // We found it.
            return entry->key;
        }

        index = (index + 1) & table->capacityMask;
        psl++;
    }
}

void tableRemoveWhite(VM *vm, Table *table) {
    for (int i = 0; i <= table->capacityMask; i++) {
        Entry *entry = &table->entries[i];
        if (entry->key != NULL && !entry->key->obj.isDark) {
            tableDelete(vm, table, entry->key);
        }
    }
}

void grayTable(VM *vm, Table *table) {
    for (int i = 0; i <= table->capacityMask; i++) {
        Entry *entry = &table->entries[i];
        grayObject(vm, (Obj *) entry->key);
        grayValue(vm, entry->value);
    }
}
