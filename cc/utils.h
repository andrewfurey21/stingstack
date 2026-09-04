#ifndef CC_UTILS_H
#define CC_UTILS_H

typedef unsigned long long u64;
typedef int i32;
typedef char b8;
typedef char i8;

#define true 1
#define false 0
#define MAX_FILE_SIZE 1024 * 1024

// C library
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

// Linux
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// Helper functions
void panic_if(b8 expr,
              const char *file,
              const char *func,
              u64 line,
              const char *msg, ...);

#define PANIC_IF(expr, msg, ...) \
  panic_if(expr, __FILE__, __func__, __LINE__, msg, ##__VA_ARGS__, NULL)

#define PANIC(msg, ...) \
  panic_if(true, __FILE__, __func__, __LINE__, msg, ##__VA_ARGS__, NULL)

// NOTE: Should be safe to assume that it returns a block of page-aligned memory.
void *mem_alloc(u64 num_bytes);
void  mem_dealloc(void *mem, u64 num_bytes);

// Linux file read helpers
void linux_file_open(const char *file_path,
                     i32 *file_descriptor,
                     u64 *num_bytes);

void linux_file_close(i32 *file_descriptor);
#endif
