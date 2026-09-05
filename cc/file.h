#ifndef CC_FILE_H
#define CC_FILE_H

#include "utils.h"

typedef struct {
  i8 *data;
  u32 num_bytes;
} file;

void file_read(file *f, const char *file_path, void *buffer);
void file_close(file *f);

#endif
