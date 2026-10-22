#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG 1
#define DEBUG_BYTE_VALUE 0xDB
#define DEBUG_PTR_VALUE NULL

#include "includes.h"
#include "defines.h"
#include "typedefs.h"

#define STACK_DEPTH 10

struct Call_Stack {
  const char *function_names[STACK_DEPTH];
  u32 lines[STACK_DEPTH];
  u32 depth;
};

#ifdef DEBUG
static struct Call_Stack call_stack = { 0 };
#endif

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

    if (crash_program) {
      printf("Call Stack:\n");
      for (u32 i = 0; i < call_stack.depth; i++) {
        printf("Line %d:%s\n", call_stack.lines[i], call_stack.function_names[i]);
      }
      exit(-1);
    }
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

#define debug_clear_array(memory, size)    \
    if (DEBUG) {                           \
      const u32 s_ = (size);               \
      for (u32 di_ = 0; di_ < s_; di_++) { \
        (memory)[di_] = DEBUG_BYTE_VALUE;  \
      }                                    \
    }

typedef enum {
  SUCCESS, FAILED, NO_ERROR_YET, FORBIDDEN
} error_type;

static inline void call_or_return(const char *name, u32 line, b8 call) {
  PANIC_IF(call_stack.depth >= STACK_DEPTH, "Call stack too deep.");
  if (call) {
    call_stack.function_names[call_stack.depth] = name;
    call_stack.lines[call_stack.depth] = line;
    call_stack.depth++;
  } else if (call_stack.depth > 0) {
    call_stack.depth--;
  }
}

#ifdef DEBUG
#define CALL call_or_return(__FUNCTION__, __LINE__, true)
#define RETURN call_or_return(__FUNCTION__, __LINE__, false); return
#define RETURN_VAL(x) call_or_return(__FUNCTION__, __LINE__, false); return (x)
#else
#define CALL
#define RETURN(x) return (x)
#endif

typedef struct {
  // TODO: make this a format_string or something. string_builder
  const char *message;
  error_type type;
} error;


static inline void error_set(error *e, error_type type, const char *message) {
  CALL;
  e->message = message;
  e->type = type;
  RETURN;
}

#endif

