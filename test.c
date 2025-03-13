#include "arena_alloc/arena_alloc.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(__linux__)
#define SIZE_T_FMT "%lu"
#elif defined(_WIN32)
#define SIZE_T_FMT "%llu"
#endif

int main(void)
{
    printf("Creating arena...\n");
    Arena *const arena = CreateArenaWithPageSize(sizeof(int) * 20);
    if (!arena)
    {
        fprintf(stderr, "Failed to allocate arena\n");
        return EXIT_FAILURE;
    }
    DebugArena(arena);

    printf("Allocating "SIZE_T_FMT" bytes...\n", sizeof(int) * 5);
    int *const arr = ArenaAlloc(arena, sizeof(int) * 5);
    if (!arr)
    {
        fprintf(stderr, "Failed to allocate bytes\n");
        FreeArena(arena);
        return EXIT_FAILURE;
    }
    DebugArena(arena);

    ResetArena(arena);
    DebugArena(arena);

    FreeArena(arena);
    return EXIT_SUCCESS;
}
