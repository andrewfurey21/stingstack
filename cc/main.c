
#include "utils.h"
#include "alloc.h"
#include "file.h"
#include "lexer.h"

int main(int argc, char **argv) {

  // const char *compilation_unit_paths[] = {
  //   "examples/empty.c",
  //   "examples/whitespace_and_comments.c",
  //   "examples/whitespace_and_comments2.c",
  //   "shouldn't work",
  //   "examples/whitespace_and_comments3.c",
  // };

  GlobalArena global_arena;
  global_arena_clear(&global_arena);

  // for (u32 i = 0; i < STATIC_ARRAY_LENGTH(compilation_unit_paths); i++) {
  //   NOTE("Compiling \"%s\"...", compilation_unit_paths[i]);
  //   global_arena_clear(&global_arena);

  //   file f = { 0 };
  //   file_copy_into_memory(&f,
  //                         compilation_unit_paths[i],
  //                         &global_arena);

  //   Tokens tokens;
  //   tokenize(&tokens, &f, &global_arena);
  //   const b8 num_token_errors = tokens_display_errors(&tokens);

  //   printf("-------------------------------------\n");
  // }

  file_write compiled;
  void *compiled_buffer =
    global_arena_aligned_alloc(&global_arena, 1024, alignof(i8));
  file_write_setup(&compiled, compiled_buffer, "finished.asm", 1024, true);

  const char *write_this = "push rbp ; this is some x86 :O";
  file_write_append_bytes(&compiled, write_this, strlen(write_this));
  file_flush_to_disk(&compiled);

  return 0;
}
