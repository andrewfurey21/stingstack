
#include "utils.h"
#include "file.h"

int main() {

  file f;
  file_read(&f, "utils.c");

  printf("Size: %llu\n", f.num_bytes);
  for (u64 i = 0; i < f.num_bytes; i++) {
    printf("%c", f.data[i]);
  }

  file_close(&f);
  return 0;
}
