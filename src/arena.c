#include "internal_common.h"
#include <ccontrol/arena.h>
#include <stdint.h>

struct ControlArena
{
    uint8_t *buffer;
    size_t capacity;
    size_t offset;
};

ControlArena *ControlArena_Create(void *backing_buffer, size_t capacity)
{
    ControlArena *arena = (ControlArena *)backing_buffer;
    arena->buffer = (uint8_t *)backing_buffer + sizeof(ControlArena);
    arena->capacity = capacity - sizeof(ControlArena);
    arena->offset = 0;

    return arena;
}

void ControlArena_Clear(ControlArena *arena) { arena->offset = 0; }

void *ArenaAlloc(ControlArena *a, size_t size)
{
    size_t align_size = ALIGN_UP(size, CCONTROL_ARENA_ALIGN_SIZE);
    if (a->offset + align_size <= a->capacity)
    {
        void *ptr = &a->buffer[a->offset];
        a->offset += align_size;
        return ptr;
    }

    return NULL;
}

void ArenaReset(ControlArena *a) { a->offset = 0; }
