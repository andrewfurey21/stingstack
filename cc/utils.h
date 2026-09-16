#ifndef CC_UTILS_H
#define CC_UTILS_H

typedef unsigned long long u64;
typedef int i32;
typedef char b8;
typedef char i8;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#define true 1
#define false 0

// Compiler configs
#define CC_DEBUG
#define CC_GLOBAL_ARENA_SIZE 1024 * 128
#define CC_MAX_FILE_NUM_BYTES 512
#define CC_DEBUG_BYTE_VALUE 0xDE
#define CC_DEBUG_PTR_VALUE NULL
#define CC_MAX_TOKEN_LENGTH 255
#define CC_MAX_NUM_TOKENS 10
#define CC_MAX_TOKEN_BUFFER_LENGTH CC_MAX_TOKEN_LENGTH * CC_MAX_NUM_TOKENS
#define CC_NEW_LINE_CHAR '\n'
#define CC_END_OF_FILE_CHAR -1
#define CC_COLUMN_LIMIT 255
#define CC_LINE_LIMIT 255

// Math related defies
#define CC_MAX_U32_VALUE 0xffffffff

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

#define RED(x) "\033[31m" x "\033[0m"
#define GREEN(x) "\033[32m" x "\033[0m"

void panic_or_note_if(const b8 expr,
                      const b8 crash_program,
                      const char *file,
                      const char *func,
                      const u32 line,
                      const char *fmt, ...);

#define PANIC_IF(expr, fmt, ...) \
  panic_or_note_if(expr, true, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__, NULL)

#define NOTE_IF(expr, fmt, ...) \
  panic_or_note_if(expr, false, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__, NULL)

#define PANIC(fmt, ...) \
  panic_or_note_if(true, true, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__, NULL)

#define NOTE(fmt, ...) \
  panic_or_note_if(true, false, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__, NULL)

b8 is_new_line(i8 byte);

#define STATIC_ARRAY_LENGTH(arr) ((sizeof(arr) / sizeof(arr[0])))

typedef enum {
  SUCCESS, FAILED, NO_ERROR_YET, FORBIDDEN
} error_type;

//  TODO: stack unwinding with an array of messages. also fix error messages.
typedef struct {
  const char *message;
  error_type type;
} error;

void error_set(error *e, const char *message, error_type type);

// Linux file read/write helpers
void linux_file_open(const char *file_path,
                     u32 flags,
                     i32 *file_descriptor,
                     u32 *num_bytes,
                     error *e);

void linux_file_read(i32 file_descriptor, void *data, u32 num_bytes, error *e);
void linux_file_write(i32 file_descriptor, void *data, u32 num_bytes, error *e);
void linux_file_close(i32 file_descriptor);

// Linux page alloc helpers
#define LINUX_PAGE_SIZE sysconf(_SC_PAGE_SIZE)
void *linux_page_alloc(u32 num_bytes);
void  linux_page_dealloc(void *mem, u32 num_bytes);

// Math
b8 is_power_of_two(u32 number);

// Portability
#define PAGE_SIZE LINUX_PAGE_SIZE
#define os_file_open linux_file_open
#define os_file_read linux_file_read
#define os_file_write linux_file_write
#define os_file_close linux_file_close

#endif
