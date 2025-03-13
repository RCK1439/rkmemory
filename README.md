# Arena-Alloc
An arena is a form of [region based memory allocation](https://en.wikipedia.org/wiki/Region-based_memory_management). <b>arena-alloc</b> is a library that provides an interface for an arena allocator built entirely in C, for C/C++.

## Usage

```c
#include <stdio.h>
#include <arena_alloc/arena_alloc.h>

int main(void)
{
    // Creates an arena with a page size of 8KB
    Arena *arena = CreateArena();

    // Allocate an array of ten integers all initialized to 0
    int *numbers = (int *)ArenaAllocZeroed(arena, sizeof(int) * 1024);

    // Do some things with the array of integers
    for (int i = 0; i < 1024; i++)
    {
        printf("numbers[%d]=%d\n", i, numbers[i]);
    }

    // Free the entire arena and all the data allocated within it
    FreeArena(arena);
}
```

Very easy!

## Authors
- Ruan C. Keet
  2025-03-13