#ifndef CC_FILE_H
#define CC_FILE_H

#include "utils.h"
#include "alloc.h"

typedef struct {
  char *data;
  const char *path;
  u32 capacity;
  u32 num_bytes;
  error file_error;
} file_read;

#define FILE_PANIC_IF_ERROR(f) \
  PANIC_IF(f->file_error.type == FAILED || f->file_error.type == FORBIDDEN, \
           "%s:\"%s\"", \
          (f->path == NULL ? "text/memory" : f->path), \
          f->file_error.message);

void file_read_setup_memory(file_read *f, void *buffer, u32 capacity);
void file_read_change_path(file_read *f, const char *path);
void file_read_into_memory(file_read *f);
void file_read_display_error(file_read *f);

i8 file_read_byte(file_read *f, u32 byte_index);
i8 file_read_byte_or_eof(file_read *f, u32 byte_index);
b8 file_read_at_end(file_read *f, u32 byte_index);
void file_read_close(file_read *f);
b8 file_read_failed(file_read *f);

typedef struct {
  char *data;
  const char *path;
  u32 capacity;
  u32 size;
  error file_error;
} file_write;

void file_write_setup_memory(file_write *fw, void *buffer, u32 capacity);
void file_write_change_path(file_write *fw, const char *path);
void file_write_string(file_write *fw, const char *buffer, u32 string_length);
void file_write_to_disk(file_write *fw);

typedef struct {
  u32 line;
  u32 column;
} file_location;

void file_location_add_new_lines(file_location *location, u32 num_new_lines);
void file_location_add_new_line(file_location *location);
void file_location_next_column(file_location *location);
void file_location_next(file_location *location, i8 current_byte);
void file_location_move_until(file_location *location, file_read *f, u32 start_index, u32 num_bytes);

#endif
