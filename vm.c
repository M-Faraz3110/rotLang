#include <stdio.h>

#include "common.h"
#include "debug.h"
#include "vm.h"

VM vm;

static void resetStack() { vm.stackTop = vm.stack; }

void initVM() { resetStack(); }

void freeVM() {}

void push(Value value) {
  *vm.stackTop = value;
  vm.stackTop++;
}

Value pop() {
  vm.stackTop--;
  return *vm.stackTop;
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
    printf("value: %g ", vm.chunk->constants.values[byte]);                    \
    vm.chunk->constants.values[byte];                                          \
  })

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
    case OP_RETURN: {
      printValue(pop());
      printf("\n");
      return INTERPRET_OK;
    }
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
}

InterpretResult interpret(Chunk *chunk) {
  vm.chunk = chunk;
  vm.ip = vm.chunk->code;
  return run();
}