
#include "utils.h"
#include "alloc.h"
#include "file.h"

int main(int argc, char **argv) {
  GlobalArena global_arena;

  file f;
  file_read(&f, "utils.c", global_arena.buffer);

  printf("Size: %u\n", f.num_bytes);
  for (u64 i = 0; i < f.num_bytes; i++) {
    printf("%c", f.data[i]);
  }

  file_close(&f);
  return 0;
}
