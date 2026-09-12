
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
  global_arena_clear(&global_arena);


  void *file_buffer =
    global_arena_aligned_alloc(&global_arena,
                               CC_MAX_FILE_NUM_BYTES * sizeof(i8),
                               alignof(i8));

  file_read f = { 0 };
  file_read_setup_memory(&f, file_buffer, CC_MAX_FILE_NUM_BYTES);

  void *token_buffer =
    global_arena_aligned_alloc(&global_arena,
                               CC_MAX_TOKEN_BUFFER_LENGTH * sizeof(Token),
                               alignof(i8));

  Tokens tokens = { 0 };
  tokens_setup(&tokens, &f, token_buffer, CC_MAX_NUM_TOKENS);

  for (u32 i = 0; i < STATIC_ARRAY_LENGTH(compilation_unit_paths); i++) {
    printf("-------------------------------------\n");
    NOTE("Compiling \"%s\"...", compilation_unit_paths[i]);

    file_read_change_path(&f, compilation_unit_paths[i]);
    file_read_into_memory(&f);

    if (file_read_failed(&f)) { file_read_display_error(&f); continue; }

    tokenize(&tokens);
    const b8 num_token_errors = tokens_display_errors(&tokens);

    // file_read_close(&f);
  }

  return 0;
}
