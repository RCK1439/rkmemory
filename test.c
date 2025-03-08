#include "arena_alloc/arena_alloc.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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

    printf("Allocating %llu bytes...\n", sizeof(int) * 5);
    int *const arr = ArenaAlloc(arena, sizeof(int) * 5);
    if (!arr)
    {
        fprintf(stderr, "Failed to allocate bytes\n");
        FreeArena(arena);
        return EXIT_FAILURE;
    }
    DebugArena(arena);

    FreeArena(arena);
    return EXIT_SUCCESS;
}
