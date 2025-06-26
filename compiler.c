#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

bool compile(const char *source, Chunk *chunk) {
  initScanner(source);
  int line = -1;
  advance();
  expression();
  consume(TOKEN_EOF, "Expect end of expression.");
}