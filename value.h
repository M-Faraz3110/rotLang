#ifndef clox_value_h
#define clox_value_h

#include "common.h"

typedef enum {
  VAL_BOOL,
  VAL_NIL,
  VAL_DOUBLE,
  VAL_INT,
} ValueType;

typedef struct {
  ValueType type;
  union {
    bool boolean;
    double doubleNum;
    int intNum;
  } as;
} Value;

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_DOUBLE(value) ((value).type == VAL_DOUBLE)
#define IS_INT(value) ((value).type == VAL_INT)

#define AS_BOOL(value) ((value).as.boolean)
#define AS_DOUBLE(value) ((value).as.doubleNum)
#define AS_INT(value) ((value).as.intNum)

#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL ((Value){VAL_NIL, {.intNum = 0}})
#define DOUBLE_VAL(value) ((Value){VAL_DOUBLE, {.doubleNum = value}})
#define INT_VAL(value) ((Value){VAL_INT, {.intNum = value}})

#define VAL_TYPE(value) ((value).type)

typedef struct {
  int capacity;
  int count;
  Value *values;
} ValueArray;

bool valuesEqual(Value a, Value b);
void initValueArray(ValueArray *array);
void writeValueArray(ValueArray *array, Value value);
void freeValueArray(ValueArray *array);
void printValue(Value value);

#endif