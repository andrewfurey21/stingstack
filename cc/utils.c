#include "utils.h"

void panic_if(const b8 expr,
              const char *file,
              const char *func,
              const u32 line,
              const char *fmt, ...) {
#ifdef CC_DEBUG
  va_list args;
  va_start(args, fmt);
  if (expr) {
    printf("Error in %s:%s() at line %u: ", file, func, line);
    vprintf(fmt, args);
    printf("\n");
    exit(-1);
  }
  va_end(args);
#endif
}

void *linux_page_alloc(u32 num_bytes) {
  void *const starting_address_of_new_mapping = NULL;
  const i32   protections = PROT_READ | PROT_WRITE;
  const i32   flags = MAP_PRIVATE | MAP_ANON;
  const i32   file_descriptor = 0;
  const i32   offset = 0;

  return mmap(starting_address_of_new_mapping,
              num_bytes,
              protections,
              flags,
              file_descriptor,
              offset);
}

void linux_page_dealloc(void *mem, u32 num_bytes) {
  munmap(mem, num_bytes);
}

void linux_file_open(const char *file_path,
                     i32 *file_descriptor,
                     u32 *num_bytes) {
  *file_descriptor = open(file_path, O_RDONLY);
  PANIC_IF(*file_descriptor == -1, "Could not read from file %s.", file_path);

  struct stat st;
  if (fstat(*file_descriptor, &st) == -1) {
    close(*file_descriptor);
    PANIC("Could not gather stats from %s.", file_path);
  }

  PANIC_IF(st.st_size > CC_MAX_FILE_NUM_BYTES,
           "file size of %s is %d, but maximum is %d.",
           file_path, st.st_size, CC_MAX_FILE_NUM_BYTES);

  *num_bytes = st.st_size;
}

void linux_file_close(i32 *file_descriptor) {
  close(*file_descriptor);
}

u32 linux_file_read(i32 file_descriptor,
                    i8 *data, u32 num_bytes) {
  return read(file_descriptor, data, num_bytes);
}

b8 is_power_of_two(u32 number) {
  return (number & (number - 1)) == 0;
}
