#ifndef LINUX_H
#define LINUX_H

#include "debug.h"
#include "includes.h"
#include "typedefs.h"
#include "defines.h"

// Linux file read/write helpers
static inline void linux_file_open(const char *file_path,
                                   u32 flags,
                                   i32 *file_descriptor,
                                   u32 *num_bytes,
                                   const u32 max_num_bytes,
                                   error *e) {

  *file_descriptor = open(file_path, flags, !!(flags & O_CREAT) * 0644);
  if (*file_descriptor == -1) return;

  if (num_bytes == NULL) return;

  struct stat st;
  if (fstat(*file_descriptor, &st) == -1) {
    close(*file_descriptor);
    error_set(e, FAILED, "Could not gather stats from file.");
    return;
  }

  if (st.st_size > max_num_bytes) {
    error_set(e, FAILED, "File to big.");
    return;
  }

  *num_bytes = st.st_size;
  error_set(e, SUCCESS, NULL);
}

static inline void linux_file_read(i32 file_descriptor, void *data, u32 num_bytes, error *e) {
  u32 num_bytes_read = read(file_descriptor, data, num_bytes);
  if (num_bytes_read != num_bytes) {
    error_set(e, FAILED, "Could not finish reading file.");
  } else {
    error_set(e, SUCCESS, NULL);
  }
}

static inline void linux_file_write(i32 file_descriptor, void *data, u32 num_bytes, error *e) {
  u32 num_bytes_written = write(file_descriptor, data, num_bytes);
  if (num_bytes_written != num_bytes) {
    error_set(e, FAILED, "Could not finish writing to disk.");
  } else {
    error_set(e, SUCCESS, NULL);
  }
}
static inline void linux_file_close(i32 file_descriptor) {
  close(file_descriptor);
}

// Linux page alloc helpers
#define LINUX_PAGE_SIZE sysconf(_SC_PAGE_SIZE)

static inline void *linux_page_alloc(u32 num_bytes) {
  NOTE_IF(num_bytes % LINUX_PAGE_SIZE != 0,
          "Not allocating full pages. Allocating %d bytes when page size is %d.",
          num_bytes, LINUX_PAGE_SIZE);

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

  debug_clear_array(buffer, num_bytes);
  return buffer;

}

static inline void linux_page_dealloc(void *mem, u32 num_bytes) {
  munmap(mem, num_bytes);
}

#endif
