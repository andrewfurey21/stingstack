
#include "utils.h"
#include "alloc.h"
#include "file.h"
#include "lexer.h"

int main(int argc, char **argv) {
  GlobalArena global_arena;

  file f;
  file_copy_into_memory(&f, "examples/examples.c", &global_arena);

  Tokens tokens;
  tokenize(&f, &tokens, &global_arena);

  const u32 num_token_errors = tokens_display_errors(&tokens);

  return 0;
}
