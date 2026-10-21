
#include "lexer.h"

#define MAX_NUM_TOKENS 100

int main(int argc, char **argv) {

  const char *compilation_unit_paths[] = {
    "examples/empty.c",
    "examples/whitespace_and_comments.c",
    "examples/whitespace_and_comments2.c",
    "shouldn't work",
    "examples/whitespace_and_comments3.c",
  };

  const u32 memory_size = 4 KB;
  u8 memory[memory_size];
  Arena arena = arena_setup(memory, memory_size);

  void *file_buffer = arena_alloc(&arena, MAX_FILE_NUM_BYTES, 1);
  file f;
  file_setup(&f, file_buffer, MAX_FILE_NUM_BYTES, NULL);

  void *token_buffer = arena_alloc(&arena, MAX_NUM_TOKENS * sizeof(Token), 1);
  Tokens tokens = { 0 };
  tokens_setup(&tokens, &f, token_buffer, MAX_NUM_TOKENS);

  for (u32 i = 0; i < STATIC_ARRAY_LENGTH(compilation_unit_paths); i++) {
    printf("-------------------------------------\n");
    NOTE("Compiling \"%s\"...", compilation_unit_paths[i]);

    file_change_path(&f, compilation_unit_paths[i]);
    file_read_into_memory(&f);

    if (file_failed(&f)) { file_display_error(&f); continue; }

    tokenize(&tokens);
    const b8 num_token_errors = tokens_display_errors(&tokens);

    file_close(&f);
  }

  return 0;
}
