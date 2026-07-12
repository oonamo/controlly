#ifndef _ARENA_H
#define _ARENA_H
#include <stddef.h>

typedef struct ControlArena ControlArena;

ControlArena *Control_Arena_Create(void *backing_buffer, size_t capacity);
void Control_Arena_Clear(ControlArena *arena);
size_t Control_Arena_RemainingSpace(ControlArena *arena);

void *Control_Arena_Alloc(ControlArena *a, size_t size);
void Control_Arena_Reset(ControlArena *a);
#endif
