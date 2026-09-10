
#include "utils.h"
#include "alloc.h"
#include "file.h"
#include "lexer.h"

int main(int argc, char **argv) {

  const char *compilation_unit_paths[] = {
    "examples/empty.c",
    "examples/whitespace_and_comments.c",
    "examples/whitespace_and_comments2.c",
    "shouldn't work",
    "examples/whitespace_and_comments3.c",
  };

  GlobalArena global_arena;

  for (u32 i = 0; i < STATIC_ARRAY_LENGTH(compilation_unit_paths); i++) {
    NOTE("Compiling \"%s\"...", compilation_unit_paths[i]);
    global_arena_clear(&global_arena);

    file f = { 0 };
    file_copy_into_memory(&f,
                          compilation_unit_paths[i],
                          &global_arena);

    Tokens tokens;
    tokenize(&tokens, &f, &global_arena);
    const b8 num_token_errors = tokens_display_errors(&tokens);

    printf("-------------------------------------\n");
  }
  return 0;
}
