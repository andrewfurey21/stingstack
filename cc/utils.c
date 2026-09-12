#include "utils.h"

void error_set(error *e, const char *message, error_type type) {
  e->message = message;
  e->type = type;
}

// TODO: do i really need this?
void error_copy(error *destination, error *source) {
  destination->message = source->message;
  destination->type = source->type;
}

void panic_or_note_if(const b8 expr,
                         const b8 crash_program,
                         const char *file,
                         const char *func,
                         const u32 line,
                         const char *fmt, ...) {
#ifdef CC_DEBUG
  va_list args;
  va_start(args, fmt);
  if (expr) {
    if (crash_program) { printf(RED("Error ")); }
    else { printf(GREEN("Compiler note ")); }
    printf("in %s:%s() at line %u: ", file, func, line);
    vprintf(fmt, args);
    printf("\n");
    if (crash_program) exit(-1);
  }
  va_end(args);
#endif
}

b8 is_new_line(i8 byte) {
  return byte == CC_NEW_LINE_CHAR;
}

void *linux_page_alloc(u32 num_bytes) {
  NOTE_IF(num_bytes % PAGE_SIZE != 0,
          "Not allocating full pages. Allocating %d bytes when page size is %d.",
          num_bytes, PAGE_SIZE);

  void *const starting_address_of_new_mapping = NULL;
  const i32   protections = PROT_READ | PROT_WRITE;
  const i32   flags = MAP_PRIVATE | MAP_ANON;
  const i32   file_descriptor = 0;
  const i32   offset = 0;

  i8 *buffer = (i8 *)mmap(starting_address_of_new_mapping,
                          num_bytes,
                          protections,
                          flags,
                          file_descriptor,
                          offset);

#ifdef CC_DEBUG
  for (u32 i = 0; i < num_bytes; i++) {
    buffer[i] = CC_DEBUG_BYTE_VALUE;
  }
#endif
  return buffer;
}

void linux_page_dealloc(void *mem, u32 num_bytes) {
  munmap(mem, num_bytes);
}

void linux_file_open(const char *file_path,
                     const u32 flags,
                     i32 *file_descriptor,
                     u32 *num_bytes,
                     error *e) {
  *file_descriptor = open(file_path, flags, !!(flags & O_CREAT) * 0644);
  if (*file_descriptor == -1) return;

  if (num_bytes == NULL) return;

  struct stat st;
  if (fstat(*file_descriptor, &st) == -1) {
    close(*file_descriptor);
    error_set(e, "Could not gather stats from file.", FAILED);
    return;
  }

  if (st.st_size > CC_MAX_FILE_NUM_BYTES) {
    error_set(e, "File to big.", FAILED);
    return;
  }

  *num_bytes = st.st_size;
  error_set(e, NULL, SUCCESS);
}

void linux_file_close(i32 file_descriptor) {
  close(file_descriptor);
}

void linux_file_read(i32 file_descriptor,
                    void *data, u32 num_bytes, error *e) {
  u32 num_bytes_read = read(file_descriptor, data, num_bytes);
  if (num_bytes_read != num_bytes) {
    error_set(e, "Could not finish reading file.", FAILED);
  } else {
    error_set(e, NULL, SUCCESS);
  }
}

void linux_file_write(i32 file_descriptor, void *data, u32 num_bytes, error *e) {
  u32 num_bytes_written = write(file_descriptor, data, num_bytes);
  if (num_bytes_written != num_bytes) {
    error_set(e, "Could not finish writing to disk.", FAILED);
  } else {
    error_set(e, NULL, SUCCESS);
  }
}

b8 is_power_of_two(u32 number) {
  return (number & (number - 1)) == 0;
}
