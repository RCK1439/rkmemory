#ifndef RK_ARENA_H
#define RK_ARENA_H

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

#if defined(RK_ARENA_IMPLEMENTATION)

#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// --- platform defines -------------------------------------------------------

#if defined(_WIN32) || defined(_WIN64)
#    define RK_ARENA_PLATFORM_WINDOWS
#elif defined(__linux__)
#    define RK_ARENA_PLATFORM_LINUX
#elif defined(__APPLE__)
#    define RK_ARENA_PLATFORM_APPLE
#endif /* platform detection */

#if defined(DEBUG)
#    define RK_ARENA_DEBUG
#elif defined(NDEBUG)
#    define RK_ARENA_RELEASE
#else
#    define RK_ARENA_DEBUG
#endif /* build mode */

// --- platform dependent includes --------------------------------------------

#if defined(RK_ARENA_PLATFORM_LINUX)
#include <sys/mman.h>
#elif defined(RK_ARENA_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(RK_ARENA_PLATFORM_APPLE)
#    error "Unimplemented: I don't own an apple device to test this with"
#endif

// --- constants --------------------------------------------------------------

#define DEFAULT_PAGE_SIZE (8 * 1024)

// --- macros -----------------------------------------------------------------

#if defined(RK_ARENA_DEBUG)
#define RK_ARENA_ASSERT(expr, ...) if (!(expr))\
    rkArenaPanic("Assertion Failed ("#expr"): " __VA_ARGS__)
#else
#define RK_ARENA_ASSERT(expr, ...) (void)0
#endif

// --- type definitions -------------------------------------------------------

/**
 * This struct defines an allocation page in the arena
 */
typedef struct rkAllocPage
{
    uint8_t            *region; // The memory region of this page
    size_t              offset; // The current offset into the memory region
    size_t              size;   // The capacity of the memory region
    struct rkAllocPage *next;   // A pointer to the next allocation page
} rkAllocPage;

/**
 * This struct defines the memory arena
 */
typedef struct rkArena
{
    size_t       pageSize; // The capacity of the allocation pages
    rkAllocPage *curr;     // The head of the allocation page linked list
} rkArena;

// --- function prototypes ----------------------------------------------------

/**
 * Creates a new arena
 *
 * @param[in] pageSize
 *      The capacity of the allocation page memory regions
 *
 * @return
 *      A pointer to the newly created arena, or `NULL` upon failure
 */
static rkArena *rkNewArena(size_t pageSize);

/**
 * Creates a new allocation page
 *
 * @param[in] size
 *      The capacity of the allocation page
 * @param[in] next
 *      A pointer to the next allocation page in the linked list
 */
static rkAllocPage *rkNewPage(size_t size, rkAllocPage *next);

/**
 * Allocates `numBytes` bytes of memory from `page`
 *
 * @param[in] page
 *      The allocation page to allocate the memory from
 * @param[in] numBytes
 *      The number of bytes to allocate from the allocation page
 *
 * @return
 *      A pointer to the newly allocated memory, or `NULL` upon failure
 */
static void *rkAllocFromPage(rkAllocPage *page, size_t numBytes);

/**
 * An operating system agnostic memory request function. This simply performs
 * a syscall to request memory from the kernel
 *
 * @param[in] numBytes
 *      The number of bytes to allocate
 *
 * @return
 *      A pointer to the newly allocated memory, or `NULL` upon failure
 */
static void *rkOsMalloc(size_t numBytes);

/**
 * Frees the memory allocated at `ptr` by use of an operating system syscall
 *
 * @param[in] ptr
 *      A pointer to the memory to deallocate
 * @param[in] numBytes
 *      The number of bytes to deallocate
 */
static void rkOsFree(void *ptr, size_t numBytes);

#if defined(RK_ARENA_DEBUG)
/**
 * This is just a show stopper for is something horribly goes wrong
 */
static void rkArenaPanic(const char *fmt, ...);
#endif

// --- arena interface --------------------------------------------------------

rkArena *rkCreateArena(void)
{
    return rkNewArena(DEFAULT_PAGE_SIZE);
}

rkArena *rkCreateArenaWithPageSize(size_t pageSize)
{
    return rkNewArena(pageSize);
}

void rkFreeArena(rkArena *arena)
{
    RK_ARENA_ASSERT(arena != NULL, "Cannot free a NULL arena");

    rkAllocPage *p = arena->curr;
    while (p)
    {
        rkAllocPage *const q = p->next;

        rkOsFree(p, sizeof(rkAllocPage) + sizeof(uint8_t) * p->size);
        p = q;
    }

    rkOsFree(arena, sizeof(rkArena));
}

void rkResetArena(rkArena *arena)
{
    RK_ARENA_ASSERT(arena != NULL, "Cannot reset a NULL arena");
    for (rkAllocPage *p = arena->curr; p; p = p->next)
    {
        p->offset = 0;
    }
}

void *rkArenaAlloc(rkArena *arena, size_t numBytes)
{
    RK_ARENA_ASSERT(arena != NULL, "Cannot allocate from NULL arena");

    rkAllocPage *const currPage = arena->curr;
    if (currPage->offset + numBytes > currPage->size)
    {
        rkAllocPage *const newPage = rkNewPage(arena->pageSize, currPage);
        if (!newPage)
        {
            return NULL;
        }
        else
        {
            arena->curr = newPage;
        }

        return rkAllocFromPage(newPage, numBytes);
    } 

    return rkAllocFromPage(currPage, numBytes);
}

void *rkArenaAllocZeroed(rkArena *arena, size_t numBytes)
{
    RK_ARENA_ASSERT(arena != NULL, "Cannot allocate from NULL arena");

    void *const ptr = rkArenaAlloc(arena, numBytes);
    if (!ptr)
    {
        return NULL;
    }

    memset(ptr, 0x00, numBytes);
    return ptr;
}

void *rkArenaRealloc(rkArena *arena, void *ptr, size_t oldSize, size_t newSize)
{
    RK_ARENA_ASSERT(arena != NULL, "Cannot reallocate from NULL arena");
    RK_ARENA_ASSERT(ptr != NULL, "Cannot reallocate a NULL pointer");
    RK_ARENA_ASSERT(oldSize <= newSize, "oldSize cannot be greater than newSize");

    uint8_t *const newBytes = (uint8_t *)rkArenaAlloc(arena, newSize);
    if (!newBytes)
    {
        return NULL;
    }

    const uint8_t *const oldBytes = (uint8_t *)ptr;
    for (size_t i = 0; i < oldSize; i++)
    {
        newBytes[i] = oldBytes[i];
    }

    return (void *)newBytes;
}

void rkDebugArena(const rkArena *arena)
{
    if (!arena)
    {
        printf("Arena { NULL }\n");
        return;
    }

    printf("Arena {\n");
    printf("\tpageSize=%zu\n", arena->pageSize);
    printf("\tcurr=");
    
    rkAllocPage *p = arena->curr;
    while (p)
    {
        printf("AllocPage { region=%p, offset=%zu, size=%zu } -> ", (void *)p->region, p->offset, p->size);
        p = p->next;
    }
    printf("NULL\n");
    printf("}\n");
}

// --- utility functions ------------------------------------------------------

static rkArena *rkNewArena(size_t pageSize)
{
    rkArena *const arena = (rkArena *)rkOsMalloc(sizeof(rkArena));
    if (!arena)
    {
        return NULL;
    }

    rkAllocPage *const page = rkNewPage(pageSize, NULL);
    if (!page)
    {
        return NULL;
    }

    arena->pageSize = pageSize;
    arena->curr = page;

    return arena;
}

static rkAllocPage *rkNewPage(size_t size, rkAllocPage *next)
{
    RK_ARENA_ASSERT(size > 0, "Page size cannot be zero");

    const size_t numBytes = sizeof(rkAllocPage) + sizeof(uint8_t) * size;
    rkAllocPage *const page = (rkAllocPage *)rkOsMalloc(numBytes);
    if (!page)
    {
        return NULL;
    }

    page->region = (uint8_t *)(page + 1);
    page->offset = 0;
    page->size = size;
    page->next = next;

    return page;
}

inline static void *rkAllocFromPage(rkAllocPage *page, size_t numBytes)
{
    RK_ARENA_ASSERT(numBytes > 0, "Cannot allocate zero bytes");
    RK_ARENA_ASSERT(numBytes <= page->size, "Cannot allocate %zu bytes from a page size of %zu bytes", numBytes, page->size);

    void *const ptr = (void *)(page->region + page->offset);
    page->offset += numBytes;

    return ptr;
}

inline static void *rkOsMalloc(size_t numBytes)
{
#if defined(RK_ARENA_PLATFORM_LINUX)
    void *const ptr = mmap(NULL, numBytes, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (ptr == MAP_FAILED)
    {
        return NULL;
    }

    return ptr;
#elif defined(RK_ARENA_PLATFORM_WINDOWS)
    void *const ptr = VirtualAllocEx(GetCurrentProcess(), NULL, numBytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (ptr == NULL || ptr == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    return ptr;
#elif defined(RK_ARENA_PLATFORM_APPLE)
#    error "Unimplemented: I don't own an apple device to test this with"
#else
    return malloc(numBytes);
#endif /* RK_ARENA_PLATFORM_XXX */
}

inline static void rkOsFree(void *ptr, size_t numBytes)
{
#if defined(RK_ARENA_PLATFORM_LINUX)
    const int r = munmap(ptr, numBytes);
    RK_ARENA_ASSERT(r == 0, "Failed to deallocate pointer: %p", ptr);
#elif defined(RK_ARENA_PLATFORM_WINDOWS)
    const BOOL r = VirtualFreeEx(GetCurrentProcess(), ptr, numBytes, MEM_RELEASE);
    RK_ARENA_ASSERT(r == FALSE, "Failed to deallocate pointer: %p", ptr);
#elif defined(RK_ARENA_PLATFORM_APPLE)
#    error "Unimplemented: I don't own an apple device to test this with"
#else
    (void)numBytes;
    free(ptr);
#endif /* RK_ARENA_PLATFORM_XXX */
}

#if defined(RK_ARENA_DEBUG)
static void rkArenaPanic(const char *fmt, ...)
{ 
    va_list args;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}
#endif /* RK_ARENA_DEBUG */
#endif /* RK_ARENA_IMPLEMENTATION */

#endif /* RK_ARENA_H */
