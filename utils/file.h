#ifndef FILE_H
#define FILE_H

#include "utils.h"

/*

File reading/writing

*/

struct file {
  const char *path;
        u8   *data;

  u32 bytes_reserved;
  u32 bytes_used;

  error err;
};

typedef struct file file;

#define FILE_PANIC_IF_ERROR(f) \
  PANIC_IF(f->err.type == FAILED || f->err.type == FORBIDDEN, \
           "%s:\"%s\"", \
          (f->path == NULL ? "text/memory" : f->path), \
          f->err.message);

static inline void file_setup(file *f, u8 *buffer, u32 bytes_reserved, const char* path) {
  f->data = buffer;
  f->path = path;
  f->bytes_reserved = bytes_reserved;
  f->bytes_used = 0;
  error_set(&f->err, NO_ERROR_YET, NULL);
}

static inline void file_change_path(file *f, const char *path) {
  f->path = path;
  f->bytes_used = 0;
  error_set(&f->err, NO_ERROR_YET, NULL);
}

static inline void file_read_into_memory(file *f) {
  i32 file_descriptor;
  error e;
  error_set(&f->err, NO_ERROR_YET, NULL);

  os_file_open(f->path, O_RDONLY, &file_descriptor, &f->bytes_used, &e);

  if (e.type == FAILED) {
    f->err = e;
    os_file_close(file_descriptor);
    return;
  }

  os_file_read(file_descriptor, f->data, f->bytes_used, &e);

  if (e.type == FAILED) {
    f->err = e;
  }

  os_file_close(file_descriptor);
}

static inline void file_display_error(file *f) {
  printf(RED("File error message") " at file \"%s\": %s\n", f->path, f->err.message);
}

static inline i8 file_read_byte(file *f, u32 byte_index) {
  FILE_PANIC_IF_ERROR(f);
  PANIC_IF(byte_index >= f->bytes_used,
           "File overflow - trying to access byte %d in file %s of length %d.",
           byte_index, f->path, f->bytes_used);
  return f->data[byte_index];
}

static inline b8 file_at_end(file *f, u32 byte_index) {
  FILE_PANIC_IF_ERROR(f);
  PANIC_IF(byte_index > f->bytes_used,
           "Index is overflowing. Index at %d while file %s is of length %d.", byte_index, f->path, f->bytes_used);
  // At end if last real byte + 1.
  return byte_index == f->bytes_used;
}

static inline i8 file_read_byte_or_eof(file *f, u32 byte_index) {
  FILE_PANIC_IF_ERROR(f);
  if (file_at_end(f, byte_index)) return END_OF_FILE_CHAR;
  return f->data[byte_index];
}

static inline void file_close(file *f) {
  FILE_PANIC_IF_ERROR(f);
  error_set(&f->err, NO_ERROR_YET, "File is closed.");
}

static inline b8 file_failed(file *f) {
  return f->err.type == FAILED;
}

static inline void file_write_to_disk(file *f) {
  FILE_PANIC_IF_ERROR(f);

  error e = { .type = SUCCESS, .message = NULL };
  i32 file_descriptor;

  os_file_open(f->path, O_RDWR | O_CREAT, &file_descriptor, NULL, &e);
  if (e.type == FAILED) {
    f->err = e;
    return;
  }

  os_file_write(file_descriptor, f->data, f->bytes_used, &e);
  os_file_close(file_descriptor);

  if (e.type == FAILED) {
    f->err = e;
  }
}

struct file_location {
  u32 line;
  u32 column;
};

typedef struct file_location file_location;

static inline void file_location_add_new_line(file_location *location) {
  PANIC_IF(location->line >= LINE_LIMIT,
           "Line limit of %d reached.", location->line);
  location->column = 1;
  location->line++;
}

static inline void file_location_next_column(file_location *location) {
  PANIC_IF(location->column >= COLUMN_LIMIT,
           "Column limit of %d reached.", location->column);
  location->column++;
}

static inline void file_location_next(file_location *location, i8 current_byte) {
  if (current_byte == '\n') {
    file_location_add_new_line(location);
  } else {
    file_location_next_column(location);
  }
}

static inline void file_location_move_until(file_location *location, file *f, u32 start_index, u32 num_bytes) {
  for (u32 current_index = 0; current_index < num_bytes; current_index++) {
    file_location_next(location, file_read_byte(f, start_index + current_index));
  }
}

#endif
