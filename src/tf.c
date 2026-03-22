#include "tf.h"
#include "arena.h"

#ifndef CONTROL_PRIVATE_API
#define CONTROL_PRIVATE_API
#endif

#ifndef CONTROL_PERSISTENT_MEMORY_SIZE
#define CONTROL_PERSISTENT_MEMORY_SIZE 1024
#endif

#ifndef CONTROL_SCRATCH_MEMORY_SIZE
#define CONTROL_SCRATCH_MEMORY_SIZE 1024
#endif


/*
 * TODO: 2 different api's
 * 1. Embedded targets
 *  a. Allows for specifying allocation technique
 *    i. If it has dynamic memory allocation, let user use it
 *    ii. Let user provide static allocation
 * 2. General Targets
 *  a. Allow user to specify allocation function
 *    i. Allows user to specify that they wish for dynamic alloctation
 */

/*
 * Used for persisten memory
 * Guaranteed to be safe up to CONTROL_PERSISTENT_MEMORY_SIZE size
 */
Arena persistent_arena;
uint8_t persistent_buffer[CONTROL_PERSISTENT_MEMORY_SIZE];

/*
 * **VOLATAILE**
 * Used for intermediate results
 * To be reset after calculations
 */
Arena scratch_arena;
uint8_t scratch_buffer[CONTROL_SCRATCH_MEMORY_SIZE];

void ControlSystemInit()
{
    ArenaInit(&scratch_arena, scratch_buffer, CONTROL_SCRATCH_MEMORY_SIZE);
    ArenaInit(&persistent_arena, persistent_buffer,
              CONTROL_PERSISTENT_MEMORY_SIZE);
}

CONTROL_PRIVATE_API control_vector_t __CreateVectorInArena(Arena *a,
                                                           size_t capacity)
{
    control_vector_t v;
    v.capacity = capacity;
    v.size = 0;

    v.coeffs = (float *)ArenaAlloc(a, capacity * sizeof(float));
    return v;
}
