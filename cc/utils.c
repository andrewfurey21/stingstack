#include "utils.h"

void panic_if(b8 expr,
              const char *file,
              const char *func,
              u64 line,
              const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  if (expr) {
    printf("Error in %s:%s at lin %llu: ", file, func, line);
    while (msg != NULL) {
      printf("%s ", msg);
      msg = va_arg(args, const char *);
    }
    printf("\n");
    exit(-1);
  }
  va_end(args);
}

void *mem_alloc(u64 num_bytes) {
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

void  mem_dealloc(void *mem, u64 num_bytes) {
  munmap(mem, num_bytes);
}

void linux_file_open(const char *file_path,
                     i32 *file_descriptor,
                     u64 *num_bytes) {
  *file_descriptor = open(file_path, O_RDONLY);
  PANIC_IF(*file_descriptor == -1, "Could not read from ", file_path);


  struct stat st;
  if (fstat(*file_descriptor, &st) == -1) {
    close(*file_descriptor);
    PANIC("Could not gather stats from ", file_path);
  }

  *num_bytes = st.st_size;
}

void linux_file_close(i32 *file_descriptor) {
  close(*file_descriptor);
}

