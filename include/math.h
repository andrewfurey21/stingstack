#ifndef MATH_H
#define MATH_H

#include "typedefs.h"

#define MAX_U32_VALUE 0xffffffff

static inline b8 is_power_of_two(u32 number) {
  return (number & (number - 1)) == 0;
}

#endif
