#include "internal_common.h"
#include <ccontrol/arena.h>
#include <stdint.h>

struct ControlArena
{
    uint8_t *buffer;
    size_t capacity;
    size_t offset;
};

ControlArena *Control_Arena_Create(void *backing_buffer, size_t capacity)
{
    ControlArena *arena = (ControlArena *)backing_buffer;
    arena->buffer = (uint8_t *)backing_buffer + sizeof(ControlArena);
    arena->capacity = capacity - sizeof(ControlArena);
    arena->offset = 0;

    return arena;
}

void Control_Arena_Clear(ControlArena *arena)
{
    arena->offset = 0;
}

size_t Control_Arena_RemainingSpace(ControlArena *arena)
{
    if (!arena)
    {
        return 0;
    }

    return arena->capacity - arena->offset;
}

void *Control_Arena_Alloc(ControlArena *a, size_t size)
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
