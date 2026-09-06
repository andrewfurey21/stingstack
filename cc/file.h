#ifndef CC_FILE_H
#define CC_FILE_H

#include "utils.h"
#include "alloc.h"

typedef struct {
  i8 *data;
  u32 num_bytes;
  const char *path;
} file;

void file_copy_into_memory(file *f, const char *file_path, GlobalArena *arena);
i8 file_read_byte(file *f, u32 byte_index);
i8 file_read_byte_or_eof(file *f, u32 byte_index);
b8 file_at_end(file *f, u32 byte_index);
void file_close(file *f);

typedef struct {
  u32 line;
  u32 column;
} file_location;

void file_location_add_new_lines(file_location *location, u32 num_new_lines);
void file_location_add_new_line(file_location *location);
void file_location_next_column(file_location *location);
void file_location_next(file_location *location, i8 current_byte);
void file_location_move_until(file *f, file_location *location, u32 start_index, u32 num_bytes);

#endif
