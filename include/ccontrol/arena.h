#ifndef _ARENA_H
#define _ARENA_H
#include <stddef.h>
#include <stdint.h>

#ifndef CCONTROL_ARENA_ALIGN_SIZE
#define CCONTROL_ARENA_ALIGN_SIZE 8
#endif

typedef struct
{
    uint8_t *_buffer;
    size_t _capacity;
    size_t _offset;
} ControlArena;

ControlArena *Control_Arena_Create(void *backing_buffer, size_t capacity);
void Control_Arena_Clear(ControlArena *arena);
size_t Control_Arena_RemainingSpace(ControlArena *arena);

void *Control_Arena_Alloc(ControlArena *a, size_t size);
#endif
