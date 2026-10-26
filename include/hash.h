#ifndef HASH_H
#define HASH_H

#include "typedefs.h"

#define DEFAULT_FNV_PRIME 16777619;
#define DEFAULT_FNV_OFFSET 2166136261;

static u32 fnv_1a_hash(i8 *bytes, u32 num_bytes) {
  u32 hash = DEFAULT_FNV_OFFSET;

  for (u32 i = 0; i < num_bytes; i++) {
    hash = hash ^ bytes[i];
    hash = hash * DEFAULT_FNV_PRIME;
  }

  return hash;
}

#endif
