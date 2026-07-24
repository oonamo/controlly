#include "internal_common.h"
#include <controlly/arena.h>

ControlArena *Control_Arena_Create(void *backing_buffer, size_t capacity)
{
    if (capacity <= sizeof(ControlArena))
    {
        return NULL;
    }
    ControlArena *arena = (ControlArena *)backing_buffer;
    arena->_buffer      = (uint8_t *)backing_buffer + sizeof(ControlArena);
    arena->_capacity    = capacity - sizeof(ControlArena);
    arena->_offset      = 0;

    return arena;
}

void Control_Arena_Clear(ControlArena *arena)
{
    arena->_offset = 0;
}

size_t Control_Arena_RemainingSpace(ControlArena *arena)
{
    if (!arena)
    {
        return 0;
    }

    return arena->_capacity - arena->_offset;
}

void *Control_Arena_Alloc(ControlArena *a, size_t size)
{
    if (size == 0)
    {
        return NULL;
    }

    size_t align_size = ALIGN_UP(size, CONTROLLY_ARENA_ALIGN_SIZE);
    if (a->_offset + align_size <= a->_capacity)
    {
        void *ptr = &a->_buffer[a->_offset];
        a->_offset += align_size;
        return ptr;
    }

    return NULL;
}
