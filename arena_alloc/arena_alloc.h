#ifndef ARENA_ALLOC_H
#define ARENA_ALLOC_H

#include <stddef.h>

typedef struct Arena Arena;

Arena *CreateArena(void);
Arena *CreateArenaWithPageSize(size_t pageSize);

void FreeArena(Arena *arena);

void *ArenaAlloc(Arena *arena, size_t numBytes);
void *ArenaAllocZeroed(Arena *arena, size_t numBytes);
void *ArenaRealloc(Arena *arena, void *ptr, size_t numBytes);

void DebugArena(const Arena *arena);

#endif /* ARENA_ALLOC_H */

