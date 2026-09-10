#include "file.h"
#include "utils.h"

void file_copy_into_memory(file *f, const char *file_path, void *buffer) {
  i32 fd;
  linux_file_open(file_path, &fd, &f->num_bytes, O_RDONLY);

  f->path = file_path;

  const u64 num_bytes_read = linux_file_read(fd, buffer, f->num_bytes);
  if (f->num_bytes != num_bytes_read) {
    f->num_bytes = 0;
    f->data = NULL;
    f->file_error.type = FAILED;
    f->file_error.message =
      "Number of bytes read was not the number expected.";
  } else {
    f->data = (const char *)buffer;
    f->file_error.type = SUCCESS;
    f->file_error.message = NULL;
  }

  linux_file_close(fd);
}

void file_set_data_to(file *f, void *buffer, u32 num_bytes) {
  f->data = (const char *)buffer;
  f->path = NULL;
  f->file_error.type = SUCCESS;
  f->file_error.message = NULL;
  f->num_bytes = num_bytes;
}

i8 file_read_byte(file *f, u32 byte_index) {
  FILE_PANIC_IF_ERROR(f);
  PANIC_IF(byte_index >= f->num_bytes,
           "File overflow - trying to access byte %d in file %s of length %d.",
           byte_index, f->path, f->num_bytes);
  return f->data[byte_index];
}

i8 file_read_byte_or_eof(file *f, u32 byte_index) {
  FILE_PANIC_IF_ERROR(f);
  if (file_at_end(f, byte_index)) return CC_END_OF_FILE_CHAR;
  return f->data[byte_index];
}

void file_close(file *f) {
  FILE_PANIC_IF_ERROR(f);
  f->data = CC_DEBUG_PTR_VALUE;
  f->num_bytes = 0;
}

b8 file_at_end(file *f, u32 byte_index) {
  FILE_PANIC_IF_ERROR(f);
  // should never be >. There's a bug if it is.
  PANIC_IF(byte_index > f->num_bytes,
           "Index is overflowing. Index at %d while file %s is of length %d.", byte_index, f->path, f->num_bytes);
  return byte_index == f->num_bytes;
}

void file_write_setup(file_write *fw, void *buffer, const char *file_path, u32 capacity, b8 overwrite) {
  fw->data = (char *)buffer;
  fw->path = file_path;
  fw->capacity = capacity;
  fw->size = 0;
  fw->file_error.type = UNKNOWN;
  fw->file_error.message = NULL;
}

void file_write_append_bytes(file_write *fw, const char *buffer, u32 num_bytes_to_write) {
  if (fw->size + num_bytes_to_write > fw->capacity) {
    fw->file_error.type = FAILED;
    fw->file_error.message = "OOM while writing. Could not finish writing to file.";
    return;
  }

  const u32 offset = fw->size;
  for (u32 i = 0; i < num_bytes_to_write; i++) {
    fw->data[i + offset] = buffer[i];
  }

  fw->size += num_bytes_to_write;
}

void file_flush_to_disk(file_write *fw) {
  FILE_PANIC_IF_ERROR(fw);

  i32 file_descriptor;
  linux_file_open(fw->path, &file_descriptor, NULL, O_RDWR | O_CREAT);
  if (file_descriptor == -1) {
    fw->file_error.type = FAILED;
    fw->file_error.message = "Could not open file when writing.";
    return;
  }

  u32 num_bytes_written = linux_file_write(file_descriptor, fw->data, fw->size);
  linux_file_close(file_descriptor);

  if (num_bytes_written != fw->size) {
    fw->file_error.type = FAILED;
    fw->file_error.message = "Could not finish writing when flushing."; // TODO: add func name to each error string.
  } else {
    fw->file_error.type = SUCCESS;
    fw->file_error.message = NULL;
  }
}

void file_location_add_new_lines(file_location *location, u32 num_new_lines) {
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

void file_location_move_until(file_location *location, file *f, u32 start_index, u32 num_bytes) {
  for (u32 current_index = 0; current_index < num_bytes; current_index++) {
    file_location_next(location, file_read_byte(f, start_index + current_index));
  }
}

