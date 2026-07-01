#ifndef _ARENA_H
#define _ARENA_H
#include <stdint.h>

typedef struct ControlArena ControlArena;

ControlArena *ControlArena_Create(void *backing_buffer, size_t capacity);
void ControlArena_Clear(ControlArena *arena);
void *ArenaAlloc(ControlArena *a, size_t size);
void ArenaReset(ControlArena *a);
#endif
