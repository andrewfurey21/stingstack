#include "file.h"
#include "utils.h"

void file_read(file *f, const char *file_path) {
  i32 fd;
  u64 num_bytes;

  linux_file_open(file_path, &fd, &num_bytes);
  PANIC_IF(num_bytes > MAX_FILE_SIZE, "file size greater than maximum.");

  f->num_bytes = num_bytes;
  f->data = (i8 *)mem_alloc(f->num_bytes);

  u64 num_bytes_read = read(fd, f->data, f->num_bytes);
  PANIC_IF(num_bytes != num_bytes_read,
           "Number of bytes read does not match the size of the file");

  linux_file_close(&fd);
}

void file_close(file *f) {
  mem_dealloc(f->data, f->num_bytes);
  f->data = NULL;
  f->num_bytes = 0;
}
