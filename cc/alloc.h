#ifndef CC_ALLOC_H
#define CC_ALLOC_H

#include "defines.h"
#include "../utils/utils.h"

// There should only be 1 global arena.
typedef struct {
  u32 position;
  i8 buffer[CC_GLOBAL_ARENA_SIZE];
} GlobalArena;

void global_arena_clear(GlobalArena *arena);
void *global_arena_aligned_alloc(GlobalArena *arena,
                                 u32 num_bytes, u32 alignment);

#endif
