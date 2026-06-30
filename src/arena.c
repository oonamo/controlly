#include <ccontrol/arena.h>
#include <stdint.h>

void ArenaInit(Arena *a, void *backing_buffer, size_t capacity)
{
    a->buffer = (uint8_t *)backing_buffer;
    a->capacity = capacity;
    a->offset = 0;
}

void *ArenaAlloc(Arena *a, size_t size)
{
    size_t align_size = (size + 3) & ~3;
    if (a->offset + align_size <= a->capacity)
    {
        void *ptr = &a->buffer[a->offset];
        a->offset += align_size;
        return ptr;
    }

    return NULL;
}

void ArenaReset(Arena *a) { a->offset = 0; }
