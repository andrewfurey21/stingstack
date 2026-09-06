#include "file.h"
#include "alloc.h"
#include "utils.h"

void file_copy_into_memory(file *f, const char *file_path, GlobalArena *arena) {
  i32 fd;

  linux_file_open(file_path, &fd, &f->num_bytes);
  f->data =
    (i8 *)global_arena_aligned_alloc(arena, f->num_bytes, alignof(i8));
  f->path = file_path;

  const u64 num_bytes_read = linux_file_read(fd, f->data, f->num_bytes);
  PANIC_IF(f->num_bytes != num_bytes_read,
           "%d bytes read from %s does not match the file size of %d bytes.",
           num_bytes_read, file_path, f->num_bytes);

  linux_file_close(&fd);
}

i8 file_read_byte(file *f, u32 byte_index) {
  PANIC_IF(byte_index >= f->num_bytes,
           "File overflow - trying to access byte %d in file %s of length %d.",
           byte_index, f->path, f->num_bytes);
  return f->data[byte_index];
}

i8 file_read_byte_or_eof(file *f, u32 byte_index) {
  if (file_at_end(f, byte_index)) return CC_END_OF_FILE_CHAR;
  return f->data[byte_index];
}

void file_close(file *f) {
  linux_page_dealloc(f->data, f->num_bytes);
  f->data = CC_DEBUG_PTR_VALUE;
  f->num_bytes = 0;
}

b8 file_at_end(file *f, u32 byte_index) {
  // should never be >. There's a bug if it is.
  PANIC_IF(byte_index > f->num_bytes,
           "Index is overflowing. Index at %d while file %s is of length %d.", byte_index, f->path, f->num_bytes);
  return byte_index == f->num_bytes;
}
void file_location_add_new_lines(file_location *location, u32 num_new_lines) {
  PANIC_IF(location->line >= CC_LINE_LIMIT - num_new_lines, "Line limit reached.");
  location->line += num_new_lines;
}

void file_location_add_new_line(file_location *location) {
  location->column = 1;
  file_location_add_new_lines(location, 1);
}

void file_location_next_column(file_location *location) {
  PANIC_IF(location->column >= CC_COLUMN_LIMIT,
           "Column limit of %d reached.", location->column);
  location->column++;
}

void file_location_next(file_location *location, i8 current_byte) {
  if (is_new_line(current_byte)) {
    file_location_add_new_line(location);
  } else {
    file_location_next_column(location);
  }
}

void file_location_move_until(file *f, file_location *location, u32 start_index, u32 num_bytes) {
  for (u32 current_index = 0; current_index < num_bytes; current_index++) {
    file_location_next(location, file_read_byte(f, start_index + current_index));
  }
}

