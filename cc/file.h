#ifndef CC_FILE_H
#define CC_FILE_H

#include "utils.h"

typedef struct {
  i8 *data;
  u64 num_bytes;
} file;

void file_read(file *f, const char *file_path);
void file_close(file *f);



#endif
