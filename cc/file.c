#include "file.h"
#include "utils.h"

void file_read_setup_memory(file_read *f, void *buffer, u32 capacity) {
  f->data = (char *)buffer;
  f->path = NULL;
  f->capacity = capacity;
  f->num_bytes = 0;
  error_set(&f->file_error, NULL, NO_ERROR_YET);
}

void file_read_change_path(file_read *f, const char *path) {
  f->path = path;
  f->num_bytes = 0;
  error_set(&f->file_error, NULL, NO_ERROR_YET);
}

void file_read_into_memory(file_read *f) {
  i32 file_descriptor;
  error e;
  error_set(&f->file_error, NULL, SUCCESS);

  os_file_open(f->path, O_RDONLY, &file_descriptor, &f->num_bytes, &e);

  if (e.type == FAILED) {
    error_copy(&f->file_error, &e);
    os_file_close(file_descriptor);
    return;
  }

  os_file_read(file_descriptor, f->data, f->num_bytes, &e);

  if (e.type == FAILED) {
    error_set(&f->file_error, "Could not finish reading.", FAILED);
  }

  os_file_close(file_descriptor);
}

void file_read_display_error(file_read *f) {
  printf(RED("File read error")" at file \"%s\": %s\n", f->path, f->file_error.message);
}

i8 file_read_byte(file_read *f, u32 byte_index) {
  FILE_PANIC_IF_ERROR(f);
  PANIC_IF(byte_index >= f->num_bytes,
           "File overflow - trying to access byte %d in file %s of length %d.",
           byte_index, f->path, f->num_bytes);
  return f->data[byte_index];
}

i8 file_read_byte_or_eof(file_read *f, u32 byte_index) {
  FILE_PANIC_IF_ERROR(f);
  if (file_read_at_end(f, byte_index)) return CC_END_OF_FILE_CHAR;
  return f->data[byte_index];
}


b8 file_read_at_end(file_read *f, u32 byte_index) {
  FILE_PANIC_IF_ERROR(f);
  PANIC_IF(byte_index > f->num_bytes,
           "Index is overflowing. Index at %d while file %s is of length %d.", byte_index, f->path, f->num_bytes);
  return byte_index == f->num_bytes;
}

void file_read_close(file_read *f) {
  FILE_PANIC_IF_ERROR(f);
  f->path = CC_DEBUG_PTR_VALUE;
  f->num_bytes = 0;
  error_set(&f->file_error, "File is closed.", FORBIDDEN);
}

b8 file_read_failed(file_read *f) {
  if (f->file_error.type == FAILED) {
    return true;
  }
  return false;
}

void file_write_setup_memory(file_write *fw, void *buffer, u32 capacity) {
  fw->data = (char *)buffer;
  fw->path = NULL;
  fw->capacity = capacity;
  fw->size = 0;
  error_set(&fw->file_error, NULL, NO_ERROR_YET);
}

void file_write_change_path(file_write *fw, const char *path) {
  FILE_PANIC_IF_ERROR(fw);
  fw->path = path;
  error_set(&fw->file_error, NULL, NO_ERROR_YET);
}

void file_write_string(file_write *fw, const char *buffer, u32 num_bytes_to_write) {
  if (fw->size + num_bytes_to_write > fw->capacity) {
    error_set(&fw->file_error, "OOM while writing a string.", FAILED);
    return;
  }

  const u32 offset = fw->size;
  for (u32 i = 0; i < num_bytes_to_write; i++) {
    fw->data[i + offset] = buffer[i];
  }

  fw->size += num_bytes_to_write;
}

void file_write_to_disk(file_write *fw) {
  FILE_PANIC_IF_ERROR(fw);

  error e = { .type = SUCCESS, .message = NULL };
  i32 file_descriptor;

  os_file_open(fw->path, O_RDWR | O_CREAT, &file_descriptor, NULL, &e);
  if (e.type == FAILED) {
    error_copy(&fw->file_error, &e);
    return;
  }

  os_file_write(file_descriptor, fw->data, fw->size, &e);
  os_file_close(file_descriptor);

  // TODO: maybe, copy if failed? add macro for returns?
  if (e.type == FAILED) {
    error_copy(&fw->file_error, &e);
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

void file_location_move_until(file_location *location, file_read *f, u32 start_index, u32 num_bytes) {
  for (u32 current_index = 0; current_index < num_bytes; current_index++) {
    file_location_next(location, file_read_byte(f, start_index + current_index));
  }
}

