#ifndef MATH_H
#define MATH_H

#include "typedefs.h"
#include "debug.h"
#include "lexing.h"

#define MAX_U32_VALUE 0xffffffff

// TODO: remove inlines?
static inline b8 is_power_of_two(u32 number) {
  return (number & (number - 1)) == 0;
}

static inline i32 ascii_hex_to_i32(i8 byte) {
  PANIC_IF(!is_hexadecimal(byte), "Can't convert %c because it's not hexadecimal.");
  return (byte & 0xf) + 9 * (byte >> 6);
}

#endif
