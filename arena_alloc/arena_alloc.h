#ifndef ARENA_ALLOC_H
#define ARENA_ALLOC_H

#include <stddef.h>

// --- type definitions -------------------------------------------------------

// Handle to the memory arena
typedef struct rkArena rkArena;

// --- arena interface --------------------------------------------------------

/**
 * Creates an arena with a page size of 8KB (`8 * 1024` bytes)
 * 
 * @return
 *      A pointer to the newly created arena, or `NULL` upon failure
 */
rkArena *rkCreateArena(void);

/**
 * Creates an arena with a specified page size
 *
 * @param[in] pageSize
 *      The size of the pages in bytes
 *
 * @return
 *      A pointer to the newly created arena, or `NULL` upon failure
 */
rkArena *rkCreateArenaWithPageSize(size_t pageSize);

/**
 * Frees the arena and all the memory allocated within it
 *
 * @param[in] arena
 *      A pointer to the arena to deallocate
 */
void rkFreeArena(rkArena *arena);

/**
 * Resets the arena marker to the beginning of the allocated pages
 *
 * @param[in] arena
 *      A pointer to the arena to reset
 */
void rkResetArena(rkArena *arena);

/**
 * Allocates `numBytes` bytes in `arena`
 *
 * @param[in] arena
 *      A pointer to the arena to allocate memory from
 * @param[in] numBytes
 *      The amount of bytes to allocate
 *
 * @return
 *      A pointer to the start of the allocated bytes, or `NULL` upon failure
 */
void *rkArenaAlloc(rkArena *arena, size_t numBytes);

/**
 * Allocates `numBytes` bytes in the `arena` and initializes the requested
 * region to `0x00`
 *
 * @param[in] arena
 *      A pointer to the arena to allocate memory from
 * @param[in] numBytes
 *      The amount of bytes to allocate
 *
 * @return
 *      A pointer to the start of the allocated bytes, or `NULL` upon failure
 */
void *rkArenaAllocZeroed(rkArena *arena, size_t numBytes);

/**
 * Resizes the region at `ptr` by `numBytes`
 *
 * @param[in] arena
 *      A pointer to the arena to reallocate in
 * @param[in] ptr
 *      A pointer to the region to reallocate
 * @param[in] oldSize
 *      The original size in bytes of the region
 * @param[in] newSize
 *      The new size in bytes of the region
 */
void *rkArenaRealloc(rkArena *arena, void *ptr, size_t oldSize, size_t newSize);

/**
 * Basic debugging function for testing use. This has to be removed before
 * making the library public
 *
 * @param[in] arena
 *      A pointer to the arena to debug
 */
void rkDebugArena(const rkArena *arena);

#endif /* ARENA_ALLOC_H */
