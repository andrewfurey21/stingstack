#ifndef STRING_H
#define STRING_H

#include "typedefs.h"

static u32 string_length(const i8 *a) {
  u32 len = 0;
  while (a[len] != 0) len++;
  return len;
}

static b8 string_equal(const i8 *a, const i8 *b, u32 length) {
  for (u32 i = 0; i < length; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}

#endif
