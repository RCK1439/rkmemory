#include "arena_alloc.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#if defined(__linux__)
#define ARENA_OS_LINUX
#include <sys/mman.h>
#elif defined(_WIN32)
#define ARENA_OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#define ARENA_OS_UNKNOWN
#endif

#define DEFAULT_PAGE_SIZE (8 * 1024)

#ifndef NDEBUG
#define ARENA_ASSERT(expr, ...) if (!(expr)) ArenaPanic("Assertion Failed ("#expr"): " __VA_ARGS__)
#else
#define ARENA_ASSERT(expr, ...) (void)0
#endif

typedef struct AllocPage
{
    uint8_t          *region;
    size_t            offset;
    size_t            size;
    struct AllocPage *next;
} AllocPage;

typedef struct Arena
{
    size_t     pageSize;
    AllocPage *curr;
} Arena;

static Arena *NewArena(size_t pageSize);
static AllocPage *NewPage(size_t size, AllocPage *next);

static void *AllocFromPage(AllocPage *page, size_t numBytes);

static void *OsMalloc(size_t numBytes);
static void OsFree(void *ptr, size_t numBytes);

#ifndef NDEBUG
static void ArenaPanic(const char *fmt, ...);
#endif

Arena *CreateArena(void)
{
    return NewArena(DEFAULT_PAGE_SIZE);
}

Arena *CreateArenaWithPageSize(size_t pageSize)
{
    return NewArena(pageSize);
}

void FreeArena(Arena *arena)
{
    if (!arena)
    {
        return;
    }

    AllocPage *p = arena->curr;
    while (p)
    {
        AllocPage *const q = p->next;

        OsFree(p, sizeof(AllocPage) + sizeof(uint8_t) * p->size);
        p = q;
    }

    OsFree(arena, sizeof(Arena));
}

void *ArenaAlloc(Arena *arena, size_t numBytes)
{
    ARENA_ASSERT(arena->pageSize >= numBytes, "Cannot allocate bytes larger than page size");
    if (!arena)
    {
        return NULL;
    }

    AllocPage *const currPage = arena->curr;
    if (currPage->offset + numBytes > currPage->size)
    {
        AllocPage *const newPage = NewPage(arena->pageSize, currPage);
        if (!newPage)
        {
            return NULL;
        }
        else
        {
            arena->curr = newPage;
        }

        return AllocFromPage(newPage, numBytes);
    } 

    return AllocFromPage(currPage, numBytes);
}

void *ArenaAllocZeroed(Arena *arena, size_t numBytes)
{
    void *const ptr = ArenaAlloc(arena, numBytes);
    if (!ptr)
    {
        return NULL;
    }

    uint8_t *const bytes = (uint8_t *)ptr;
    for (size_t i = 0; i < numBytes; i++)
    {
        bytes[i] = 0x00;
    }

    return ptr;
}

void *ArenaRealloc(Arena *arena, void *ptr, size_t numBytes)
{
    (void)arena;
    (void)ptr;
    (void)numBytes;
    return NULL;
}

void DebugArena(const Arena *arena)
{
    if (!arena)
    {
        printf("Arena { NULL }\n");
        return;
    }

    printf("Arena {\n");
    printf("\tpageSize=%lu\n", arena->pageSize);
    printf("\tcurr=");
    
    AllocPage *p = arena->curr;
    while (p)
    {
        printf(
            "AllocPage { region=%p, offset=%lu, size=%lu } -> ",
            (void *)p->region,
            p->offset,
            p->size
        );
        p = p->next;
    }
    printf("NULL\n");
    printf("}\n");
}

static Arena *NewArena(size_t pageSize)
{
    Arena *const arena = (Arena *)OsMalloc(sizeof(Arena));
    if (!arena)
    {
        return NULL;
    }

    AllocPage *const page = NewPage(pageSize, NULL);
    if (!page)
    {
        return NULL;
    }

    arena->pageSize = pageSize;
    arena->curr = page;

    return arena;
}

static AllocPage *NewPage(size_t size, AllocPage *next)
{
    ARENA_ASSERT(size > 0, "Page size cannot be zero");

    const size_t numBytes = sizeof(AllocPage) + sizeof(uint8_t) * size;
    AllocPage *const page = (AllocPage *)OsMalloc(numBytes);
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

static void *AllocFromPage(AllocPage *page, size_t numBytes)
{
    ARENA_ASSERT(numBytes > 0, "Cannot allocate zero bytes");

    void *const ptr = (void *)(page->region + page->offset);
    page->offset += numBytes;

    return ptr;
}

inline static void *OsMalloc(size_t numBytes)
{
#if defined(ARENA_OS_LINUX)
    void *const ptr = mmap(NULL, numBytes, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (ptr == MAP_FAILED)
    {
        return NULL;
    }

    return ptr;
#elif defined(ARENA_OS_WINDOWS)
    void *const ptr = VirtualAllocEx(GetCurrentProcess(), NULL, numBytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (ptr == NULL || ptr == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    return ptr;
#else
    return malloc(numBytes);
#endif
}

inline static void OsFree(void *ptr, size_t numBytes)
{
#if defined(ARENA_OS_LINUX)
    const int r = munmap(ptr, numBytes);
    ARENA_ASSERT(r == 0, "Failed to deallocate pointer: %p", ptr);
#elif defined(ARENA_OS_WINDOWS)
    const BOOL r = VirtualFreeEx(GetCurrentProcess(), (LPVOID)ptr, 0, numBytes, MEM_RELEASE);
    ARENA_ASSERT(r == FALSE, "Failed to deallocate pointer: %p", ptr);
#else
    (void)numBytes;
    free(ptr);
#endif
}

#ifndef NDEBUG
static void ArenaPanic(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}
#endif

