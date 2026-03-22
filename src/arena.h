#ifndef _ARENA_H
#define _ARENA_H
#include <stdint.h>

typedef struct
{
    uint8_t *buffer;
    size_t capacity;
    size_t offset;
} Arena;

void ArenaInit(Arena *a, void *backing_buffer, size_t capacity);
void *ArenaAlloc(Arena *a, size_t size);
void ArenaReset(Arena *a);
#endif
