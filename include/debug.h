#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG 1
#define DEBUG_BYTE_VALUE 0xDB
#define DEBUG_PTR_VALUE NULL


#include "includes.h"
#include "defines.h"
#include "typedefs.h"

typedef enum {
  SUCCESS, FAILED, NO_ERROR_YET, FORBIDDEN
} error_type;

//  TODO: stack unwinding with an array of messages. also fix error messages.
//  TODO: this would be very useful, especially for compiler dev.
//  this should be 1 static object.
//  sort of a hacky version of stack unwinding to help find bugs
//  since compilers involve a lot of deep state this is probably something i want.
//  this means allocations like malloc are just a bad idea. especially if you can recover.
// should have a define or something, for debug mode only.
// panics should print that stack trace.
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

#define debug_clear_array(memory, size)           \
  do {                                            \
    if (DEBUG) {                                  \
      const u32 s_ = (size);                      \
      for (u32 di_ = 0; di_ < s_; di_++) {        \
        (memory)[di_] = DEBUG_BYTE_VALUE;         \
        }                                         \
    }                                             \
  } while (0)

#endif

