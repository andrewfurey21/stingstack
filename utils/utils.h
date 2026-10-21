#ifndef UTILS_H
#define UTILS_H

typedef unsigned long long u64;
typedef int i32;
typedef char b8;
typedef char i8;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#define true 1
#define false 0

#define DEBUG
#define DEBUG_BYTE_VALUE 0xDB
#define DEBUG_PTR_VALUE NULL

#define MAX_FILE_NUM_BYTES 512
#define END_OF_FILE_CHAR -1
#define COLUMN_LIMIT 255
#define LINE_LIMIT 255

// C related.
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <alloca.h>
#include <string.h>
#include <stdalign.h>

// Linux related.
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// Helper functions

#define KB * 1024
#define MB KB KB
#define GB MB KB

#define RED(x) "\033[31m" x "\033[0m"
#define GREEN(x) "\033[32m" x "\033[0m"

static inline void panic_or_note_if(const b8 expr,
                                    const b8 crash_program,
                                    const char *file,
                                    const char *func,
                                    const u32 line,
                                    const char *fmt, ...) {
#ifdef DEBUG
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

#define PANIC_IF(expr, fmt, ...) \
  panic_or_note_if(expr, true, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__, NULL)

#define NOTE_IF(expr, fmt, ...) \
  panic_or_note_if(expr, false, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__, NULL)

#define PANIC(fmt, ...) \
  panic_or_note_if(true, true, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__, NULL)

#define NOTE(fmt, ...) \
  panic_or_note_if(true, false, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__, NULL)

#define STATIC_ARRAY_LENGTH(arr) ((sizeof(arr) / sizeof(arr[0])))

typedef enum {
  SUCCESS, FAILED, NO_ERROR_YET, FORBIDDEN
} error_type;

//  TODO: stack unwinding with an array of messages. also fix error messages.
//  this should be 1 static object.
//  sort of a hacky version of stack unwinding to help find bugs
//  since compilers involve a lot of deep state this is probably something i want.
//  this means allocations like malloc are just a bad idea. especially if you can recover.
// should have a define or something, for debug mode only.
typedef struct {
  // TODO: make this a format_string or something. string_builder
  const char *message;
  error_type type;
} error;

// TODO: message should be some sort of formatting string
static inline void error_set(error *e, error_type type, const char *message) {
  e->message = message;
  e->type = type;
}

// Linux file read/write helpers
static inline void linux_file_open(const char *file_path,
                     u32 flags,
                     i32 *file_descriptor,
                     u32 *num_bytes,
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

  if (st.st_size > MAX_FILE_NUM_BYTES) {
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

#ifdef DEBUG
  for (u32 i = 0; i < num_bytes; i++) {
    buffer[i] = DEBUG_BYTE_VALUE;
  }
#endif
  return buffer;

}

static inline void linux_page_dealloc(void *mem, u32 num_bytes) {
  munmap(mem, num_bytes);
}

// Math
#define MAX_U32_VALUE 0xffffffff

static inline b8 is_power_of_two(u32 number) {
  return (number & (number - 1)) == 0;
}

// Portability
#define PAGE_SIZE LINUX_PAGE_SIZE
#define os_file_open linux_file_open
#define os_file_read linux_file_read
#define os_file_write linux_file_write
#define os_file_close linux_file_close
#define os_page_alloc linux_page_alloc
#define os_page_dealloc linux_page_dealloc

#endif
