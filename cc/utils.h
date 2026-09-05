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
#define CC_GLOBAL_ARENA_SIZE 1024 * 1024
#define CC_MAX_FILE_NUM_BYTES 128 * 1024
#define CC_DEBUG
#define CC_DEBUG_BYTE_VALUE 'a'
#define CC_DEBUG_PTR_VALUE NULL
#define CC_MAX_TOKEN_LENGTH 255

// C related.
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <alloca.h>

// Linux related.
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// Helper functions
void panic_if(b8 expr,
              const char *file,
              const char *func,
              u32 line,
              const char *fmt, ...);

#define PANIC_IF(expr, fmt, ...) \
  panic_if(expr, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__, NULL)

#define PANIC(fmt, ...) \
  panic_if(true, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__, NULL)

// Linux file read helpers
void linux_file_open(const char *file_path,
                     i32 *file_descriptor,
                     u32 *num_bytes);

u32 linux_file_read(i32 file_descriptor,
                    i8 *data, u32 num_bytes);

void linux_file_close(i32 *file_descriptor);

// Linux page alloc helpers
void *linux_page_alloc(u32 num_bytes);
void  linux_page_dealloc(void *mem, u32 num_bytes);

// Math
b8 is_power_of_two(u32 number);

#endif
