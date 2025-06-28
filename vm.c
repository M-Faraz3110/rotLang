#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "vm.h"

VM vm;

static void resetStack() { vm.stackTop = vm.stack; }

static void runtimeError(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  size_t instruction = vm.ip - vm.chunk->code - 1;
  int line = vm.chunk->lines[instruction].lineNumber;
  fprintf(stderr, "[line %d] in script\n", line);
  resetStack();
}

void initVM() { resetStack(); }

void freeVM() {}

void push(Value value) {
  *vm.stackTop = value;
  vm.stackTop++;
}

void negate() {
  vm.stackTop--;
  *vm.stackTop = DOUBLE_VAL(-AS_DOUBLE(*vm.stackTop));
  vm.stackTop++;
}

Value pop() {
  vm.stackTop--;
  return *vm.stackTop;
}

static Value peek(int distance) { return vm.stackTop[-1 - distance]; }

static bool isFalsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static InterpretResult run() {
#define READ_BYTE()                                                            \
  ({                                                                           \
    uint8_t byte = *vm.ip;                                                     \
    *vm.ip++;                                                                  \
    printf("read byte: %d ", byte);                                            \
    byte;                                                                      \
  })

#define READ_CONSTANT()                                                        \
  ({                                                                           \
    uint8_t byte = READ_BYTE();                                                \
    Value val = vm.chunk->constants.values[byte];                              \
    if (IS_DOUBLE(val)) {                                                      \
      printf("value: %g ", vm.chunk->constants.values[byte].as.doubleNum);     \
    } else if (IS_INT(val)) {                                                  \
      printf("value: %d ", vm.chunk->constants.values[byte].as.intNum);        \
    }                                                                          \
    vm.chunk->constants.values[byte];                                          \
  })
#define BINARY_OP(valueType, op)                                               \
  do {                                                                         \
    Value num1 = peek(0);                                                      \
    Value num2 = peek(1);                                                      \
    if (IS_DOUBLE(num1)) {                                                     \
      if (!IS_DOUBLE(num2)) {                                                  \
        runtimeError("Operands type mismatch");                                \
        return INTERPRET_RUNTIME_ERROR;                                        \
      }                                                                        \
      double b = AS_DOUBLE(pop());                                             \
      double a = AS_DOUBLE(pop());                                             \
      push(valueType(a op b));                                                 \
    } else if (IS_INT(num1)) {                                                 \
      if (!IS_INT(num2)) {                                                     \
        runtimeError("Operands must be numbers.");                             \
        return INTERPRET_RUNTIME_ERROR;                                        \
      }                                                                        \
      int b = AS_INT(pop());                                                   \
      int a = AS_INT(pop());                                                   \
      push(valueType(a op b));                                                 \
    }                                                                          \
  } while (false)

  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    printf("          ");
    for (Value *slot = vm.stack; slot < vm.stackTop; slot++) {
      printf("[ ");
      printValue(*slot);
      printf(" ]");
    }
    printf("\n");
    disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif

    uint8_t instruction;
    switch (instruction = READ_BYTE()) {
    case OP_CONSTANT: {
      Value constant = READ_CONSTANT();
      push(constant);
      //   printValue(constant);
      printf("\n");
      break;
    }
    case OP_NIL:
      push(NIL_VAL);
      break;
    case OP_TRUE:
      push(BOOL_VAL(true));
      break;
    case OP_FALSE:
      push(BOOL_VAL(false));
      break;
    case OP_EQUAL: {
      Value b = pop();
      Value a = pop();
      push(BOOL_VAL(valuesEqual(a, b)));
      break;
    }
    case OP_GREATER:
      BINARY_OP(BOOL_VAL, >);
      break;
    case OP_LESS:
      BINARY_OP(BOOL_VAL, <);
      break;
    case OP_ADD:
      if (IS_INT(peek(0))) {
        BINARY_OP(INT_VAL, +);
      } else if (IS_DOUBLE(peek(0))) {
        BINARY_OP(DOUBLE_VAL, +);
      } else {
        runtimeError("Operands type mistmatch");
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    case OP_SUBTRACT:
      if (IS_INT(peek(0))) {
        BINARY_OP(INT_VAL, -);
      } else if (IS_DOUBLE(peek(0))) {
        BINARY_OP(DOUBLE_VAL, -);
      } else {
        runtimeError("Operands type mistmatch");
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    case OP_MULTIPLY:
      if (IS_INT(peek(0))) {
        BINARY_OP(INT_VAL, *);
      } else if (IS_DOUBLE(peek(0))) {
        BINARY_OP(DOUBLE_VAL, *);
      } else {
        runtimeError("Operands type mistmatch");
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    case OP_DIVIDE:
      if (IS_INT(peek(0))) {
        BINARY_OP(INT_VAL, /);
      } else if (IS_DOUBLE(peek(0))) {
        BINARY_OP(DOUBLE_VAL, /);
      } else {
        runtimeError("Operands type mistmatch");
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    case OP_NOT:
      push(BOOL_VAL(isFalsey(pop())));
      break;
    case OP_NEGATE:
      if (!IS_DOUBLE(peek(0)) && !IS_INT(peek(0))) {
        runtimeError("Operand must be a number.");
        return INTERPRET_RUNTIME_ERROR;
      }
      // push(NUMBER_VAL(-AS_NUMBER(pop())));
      negate();
      break;
      //   clock_t t1 = clock();
      //   // push(-pop());
      //   negate();
      //   clock_t t2 = clock();
      //   double time_taken = t2 - t1;
      //   printf("time taken: %f \n", time_taken / CLOCKS_PER_SEC);
      //   break;

    case OP_RETURN: {
      printValue(pop());
      printf("\n");
      return INTERPRET_OK;
    }
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(const char *source) {
  Chunk chunk;
  initChunk(&chunk);

  if (!compile(source, &chunk)) {
    freeChunk(&chunk);
    return INTERPRET_COMPILE_ERROR;
  }

  vm.chunk = &chunk;
  vm.ip = vm.chunk->code;

  InterpretResult result = run();

  freeChunk(&chunk);
  return result;
}