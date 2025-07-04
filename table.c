#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

void initTable(Table *table) {
  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;
}

void freeTable(Table *table) {
  FREE_ARRAY(Entry, table->entries, table->capacity);
  initTable(table);
}

uint32_t getHashValue(Value value) {
  switch (value.type) {
  case VAL_BOOL: {
    return AS_BOOL(value) ? 1 : 0;
  }
  case VAL_NIL: {
    return 2;
  }
  case VAL_INT: {
    int64_t i = AS_INT(value);
    return (uint32_t)(i ^ (i >> 32));
  }
  case VAL_DOUBLE: {
    double d = AS_DOUBLE(value);
    uint64_t bits;
    memcpy(&bits, &d, sizeof(double));
    return (uint32_t)(bits ^ (bits >> 32));
  }
  case VAL_OBJ: {
    Obj *obj = AS_OBJ(value);
    if (obj->type == OBJ_STRING) {
      return ((ObjString *)obj)->hash;
    }
    return 3;
  }

  default:
    return 0;
  }
}

static Entry *findEntry(Entry *entries, int capacity, Value key) {
  uint32_t hash = getHashValue(key);
  uint32_t index = hash % capacity;
  Entry *tombstone = NULL;

  for (;;) {
    Entry *entry = &entries[index];
    if (valuesEqual(entry->key, NIL_VAL)) {
      if (IS_NIL(entry->value)) {
        // Empty entry.
        return tombstone != NULL ? tombstone : entry;
      } else {
        // We found a tombstone.
        if (tombstone == NULL)
          tombstone = entry;
      }
    } else if (valuesEqual(entry->key, key)) {
      // We found the key.
      return entry;
    }

    index = (index + 1) % capacity;
  }
}

static void adjustCapacity(Table *table, int capacity) {
  Entry *entries = ALLOCATE(Entry, capacity);
  for (int i = 0; i < capacity; i++) {
    entries[i].key = NIL_VAL;
    entries[i].value = NIL_VAL;
  }

  table->count = 0;
  for (int i = 0; i < table->capacity; i++) {
    Entry *entry = &table->entries[i];
    if (valuesEqual(entry->key, NIL_VAL))
      continue;

    Entry *dest = findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    table->count++;
  }

  FREE_ARRAY(Entry, table->entries, table->capacity);

  table->entries = entries;
  table->capacity = capacity;
}

bool tableGet(Table *table, Value key, Value *value) {
  if (table->count == 0)
    return false;

  Entry *entry = findEntry(table->entries, table->capacity, key);
  if (valuesEqual(entry->key, NIL_VAL))
    return false;

  *value = entry->value;
  return true;
}

bool tableSet(Table *table, Value key, Value value) {
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    int capacity = INCREASE_CAPACITY(table->capacity);
    adjustCapacity(table, capacity);
  }
  Entry *entry = findEntry(table->entries, table->capacity, key);
  bool isNewKey = valuesEqual(entry->key, NIL_VAL);
  if (isNewKey && IS_NIL(entry->value))
    table->count++;

  entry->key = key;
  entry->value = value;
  return isNewKey;
}

bool tableDelete(Table *table, Value key) {
  if (table->count == 0)
    return false;

  // Find the entry.
  Entry *entry = findEntry(table->entries, table->capacity, key);
  if (valuesEqual(entry->key, NIL_VAL))
    return false;

  // Place a tombstone in the entry.
  entry->key = NIL_VAL;
  entry->value = BOOL_VAL(true);
  return true;
}

void tableAddAll(Table *from, Table *to) {
  for (int i = 0; i < from->capacity; i++) {
    Entry *entry = &from->entries[i];
    if (!valuesEqual(entry->key, NIL_VAL)) {
      tableSet(to, entry->key, entry->value);
    }
  }
}

ObjString *tableFindString(Table *table, const char *chars, int length,
                           uint32_t hash) {
  if (table->count == 0)
    return NULL;

  uint32_t index = hash % table->capacity;
  for (;;) {
    Entry *entry = &table->entries[index];
    if (valuesEqual(entry->key, NIL_VAL)) {
      // Stop if we find an empty non-tombstone entry.
      if (IS_NIL(entry->value))
        return NULL;
    } else {
      if (IS_STRING(entry->key)) {
        if (AS_STRING(entry->key)->length == length &&
            AS_STRING(entry->key)->hash == hash &&
            memcmp(AS_STRING(entry->key)->chars, chars, length) == 0) {
          // We found it.
          return entry->key.as.obj;
        }
      }
    }

    index = (index + 1) % table->capacity;
  }
}
