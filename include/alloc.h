#ifndef ALLOC_H
#define ALLOC_H

#include "typedefs.h"
#include "debug.h"
#include "math.h"

struct Arena {
  u32 bytes_used;
  u32 bytes_reserved;
  u8  *memory;
};

typedef struct Arena Arena;

static inline Arena arena_setup(u8 *memory, u32 bytes_reserved) {
  return (Arena) {
    .bytes_used     = 0,
    .bytes_reserved = bytes_reserved,
    .memory         = memory,
  };
}

static inline void arena_clear(Arena *arena) {
  debug_clear_array(arena->memory, arena->bytes_reserved);
  arena->bytes_used = 0;
}

static inline void *arena_alloc(Arena *arena, u32 num_bytes, u32 alignment) {
  CALL;

  PANIC_IF(!is_power_of_two(alignment),
           "Cannot have alignment of %d, must be a power of 2.", alignment);

  PANIC_IF(arena->bytes_used + num_bytes > arena->bytes_reserved,
           "Could not make allocation at %d of size %d when only reserved %d.",
           arena->bytes_used, num_bytes, arena->bytes_reserved);

  const u32 address     = (u32)(uptr)(arena->memory + arena->bytes_used);
  const b8  not_aligned  = !!(address & (alignment - 1));
  const u32 extra_bytes = (alignment - address & (alignment - 1));

  arena->bytes_used += (not_aligned) * extra_bytes;
  void *memory = arena->memory + arena->bytes_used;
  arena->bytes_used += num_bytes;

  PANIC_IF((uptr)(memory) % alignment != 0, "Something went wrong with alignment when allocating in arena.");

  RETURN_VAL(memory);
}

#endif
