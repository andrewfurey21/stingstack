#include "file.h"

void file_read(file *f, const char *file_path, void *buffer) {
  i32 fd;

  f->data = (i8 *)buffer;
  linux_file_open(file_path, &fd, &f->num_bytes);

  const u64 num_bytes_read = linux_file_read(fd, f->data, f->num_bytes);
  PANIC_IF(f->num_bytes != num_bytes_read,
           "%d bytes read from %s does not match the file size of %d bytes.",
           num_bytes_read, file_path, f->num_bytes);

  linux_file_close(&fd);
}

void file_close(file *f) {
  linux_page_dealloc(f->data, f->num_bytes);
#ifdef CC_DEBUG
  f->data = CC_DEBUG_PTR_VALUE;
  f->num_bytes = 0;
#endif
}
