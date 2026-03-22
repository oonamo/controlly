#include "tf.h"
#include "arena.h"

#ifndef CONTROL_PRIVATE_API
#define CONTROL_PRIVATE_API static
#endif

#ifndef CONTROL_INLINE_API
#define CONTROL_INLINE_API inline
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

#define NULL_VECTOR                                                            \
    (control_vector_t) { .capacity = 0, .size = 0, .coeffs = NULL }

CONTROL_PRIVATE_API control_vector_t __CreateVectorInArena(Arena *a,
                                                           size_t capacity)
{
    control_vector_t v;
    v.capacity = capacity;
    v.size = 0;

    v.coeffs = (float *)ArenaAlloc(a, capacity * sizeof(float));
    return v;
}

/**
 * @brief Creates a vector of polynomial coefficients
 *
 * @param coeffs Polynomial coefficients starting from n -> 0
 * @param size size of coefficients
 * @return Vector representation
 */
control_vector_t PolyCoeffVector(float *coeffs, size_t size)
{
    if (coeffs == NULL || size <= 0)
    {
        return NULL_VECTOR;
    }

    control_vector_t v = __CreateVectorInArena(&persistent_arena, size);
    v.size = size;

    for (size_t i = 0; i < size; i++)
    {
        v.coeffs[i] = coeffs[i];
    }

    return v;
}

CONTROL_PRIVATE_API char __UnsafeControlsIntToChar(int i)
{
    return (char)(i + '0');
}

CONTROL_INLINE_API CONTROL_PRIVATE_API int
__ControlsAppendChar(char c, char *buffer, size_t buffer_size)
{
    if (buffer_size + 1 < buffer_size)
    {
        return 0;
    }
    *(buffer + 1) = c;
    return 1;
}

int PolyCoeffVectorToStr(control_vector_t *coeffs, char var, char *buffer,
                         size_t buffer_size)
{
    int order = (int)(coeffs->size) - 1;
    size_t buffer_ptr = 0;

    while (order >= 0 && buffer_ptr < buffer_size)
    {
        buffer[buffer_ptr++] = var;
        buffer[buffer_ptr++] = '^';
        buffer[buffer_ptr++] = __UnsafeControlsIntToChar(order);

        if (order != 0)
        {
            buffer[buffer_ptr++] = '+';
        }
        order--;
    }

    if (buffer_ptr < buffer_size)
    {
        buffer[buffer_ptr] = '\0';
    }

    return order != 0;
}
