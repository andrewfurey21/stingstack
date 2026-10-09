#include "alloc.h"

void global_arena_clear(GlobalArena *arena) {
#ifdef CC_DEBUG
  for (u64 i = 0; i < CC_GLOBAL_ARENA_SIZE; i++) {
    arena->buffer[i] = CC_DEBUG_BYTE_VALUE;
  }
#endif
  arena->position = 0;
}

void *global_arena_aligned_alloc(GlobalArena *arena,
                                 u32 num_bytes, u32 alignment) {
  PANIC_IF(alignment <= 0 && !is_power_of_two(alignment),
           "Cannot have alignment of %d, must be a power"
           " of two and greater than 0.", alignment);

  // trying to be branchless :)
  // TODO: why did i negate here?
  b8 is_aligned = (!!(arena->position & ~(alignment - 1)));
  u32 offset =  (alignment - arena->position & (alignment - 1));

  PANIC_IF(arena->position + num_bytes > CC_GLOBAL_ARENA_SIZE,
           "Could not make allocation at %d with offset %d of size %d when max is %d.",
           arena->position, is_aligned * offset, num_bytes, CC_GLOBAL_ARENA_SIZE);

  arena->position += is_aligned * offset;
  void *memory = arena->buffer + arena->position;
  arena->position += num_bytes;
  return memory;
}

