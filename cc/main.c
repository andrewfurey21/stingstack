#include "../utils/utils.h"  // TODO: make a sting.h that has all utils these included.
#include "../utils/file.h"

#include "defines.h"
#include "alloc.h"
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
                               MAX_FILE_NUM_BYTES * sizeof(i8),
                               alignof(i8));

  file f = { 0 };
  file_setup(&f, file_buffer, MAX_FILE_NUM_BYTES, NULL);

  // TODO: remove this global arena shite. make a u8 buffer[length], pass to some arena struct.
  void *token_buffer =
    global_arena_aligned_alloc(&global_arena,
                               CC_MAX_TOKEN_BUFFER_LENGTH * sizeof(Token),
                               alignof(i8));

  Tokens tokens = { 0 };
  tokens_setup(&tokens, &f, token_buffer, CC_MAX_NUM_TOKENS);

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
