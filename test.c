#define RK_ARENA_IMPLEMENTATION
#include "rkmemory/rkarena.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    printf("Creating arena...\n");
    rkArena *const arena = rkCreateArenaWithPageSize(sizeof(int) * 20);
    if (!arena)
    {
        fprintf(stderr, "Failed to allocate arena\n");
        return EXIT_FAILURE;
    }
    rkDebugArena(arena);

    printf("Allocating %zu bytes...\n", sizeof(int) * 5);
    int *const arr = rkArenaAlloc(arena, sizeof(int) * 5);
    if (!arr)
    {
        fprintf(stderr, "Failed to allocate bytes\n");
        rkFreeArena(arena);
        return EXIT_FAILURE;
    }
    rkDebugArena(arena);

    rkResetArena(arena);
    rkDebugArena(arena);

    rkFreeArena(arena);
    return EXIT_SUCCESS;
}
