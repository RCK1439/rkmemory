# rkmemory
An arena is a form of [region based memory allocation](https://en.wikipedia.org/wiki/Region-based_memory_management). <b>rkmemory</b> is a library that provides an interface for an arena allocator built entirely in C, for C/C++.

## Usage

```c
#include <stdio.h>
#define RK_ARENA_IMPLEMENTATION
#include <rkmemory/rkarena.h>

int main(void)
{
    // Creates an arena with a page size of 8KB
    rkArena *arena = rkCreateArena();

    // Allocate an array of ten integers all initialized to 0
    int *numbers = (int *)rkArenaAllocZeroed(arena, sizeof(int) * 1024);

    // Do some things with the array of integers
    for (int i = 0; i < 1024; i++)
    {
        printf("numbers[%d]=%d\n", i, numbers[i]);
    }

    // Free the entire arena and all the data allocated within it
    rkFreeArena(arena);
}
```

Very easy!

## Authors
- Ruan C. Keet
  2025-03-13
