#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"

void initValueArray(ValueArray *array) {
  array->values = NULL;
  array->capacity = 0;
  array->count = 0;
}

void writeValueArray(ValueArray *array, Value value) {
  if (array->capacity < array->count + 1) {
    int oldCapacity = array->capacity;
    array->capacity = INCREASE_CAPACITY(oldCapacity);
    array->values =
        INCREASE_ARRAY(Value, array->values, oldCapacity, array->capacity);
  }

  array->values[array->count] = value;
  array->count++;
}

void freeValueArray(ValueArray *array) {
  FREE_ARRAY(Value, array->values, array->capacity);
  initValueArray(array);
}

void printValue(Value value) {
  switch (value.type) {
  case VAL_BOOL:
    printf(AS_BOOL(value) ? "true" : "false");
    break;
  case VAL_NIL:
    printf("nil");
    break;
  case VAL_DOUBLE:
    printf("%g", AS_DOUBLE(value));
    break;
  case VAL_INT:
    printf("%d", AS_INT(value));
    break;
  case VAL_OBJ:
    printObject(value);
    break;
  }
}

bool valuesEqual(Value a, Value b) {
  if (a.type != b.type)
    return false;
  switch (a.type) {
  case VAL_BOOL:
    return AS_BOOL(a) == AS_BOOL(b);
  case VAL_NIL:
    return true;
  case VAL_DOUBLE:
    return AS_DOUBLE(a) == AS_DOUBLE(b);
  case VAL_INT:
    return AS_INT(a) == AS_INT(b);
  case VAL_OBJ:
    return AS_OBJ(a) == AS_OBJ(b); // cuz of string interning
  default:
    return false; // Unreachable.
  }
}