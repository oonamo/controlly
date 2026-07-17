/**
 * @file arena.h
 * @brief Lightweight memory implementation
 *
 * This module contains a fast O(1) bump allocator which
 * is designed to prevent memory fragmentation
 * The arena can be used standalone, without the need of any other header
 */
#ifndef _ARENA_H
#define _ARENA_H
#include <stddef.h>
#include <stdint.h>

#ifndef CCONTROL_ARENA_ALIGN_SIZE
#define CCONTROL_ARENA_ALIGN_SIZE 8
#endif

/**
 * @brief In-place structure for the memory arena
 *
 * @warning This struct allocates itself into the backing buffer.
 * It is possibile that `sizeof(ControlArena) != Control_Arena_RemainingSpace(arena)`
 *
 * @warning Fields prefixed with an underscore (_) are considered private
 * and should not be modified by the user
 */
typedef struct
{
    uint8_t *_buffer;
    size_t _capacity;
    size_t _offset;
} ControlArena;

/**
 * @brief Initializes a new memory arena inside the provided backing buffer.
 *
 * This is an in-place allocator. The `ControlArena` struct is placed in the beginning
 * of the provided buffer. The actual usable capacity at init will be `capacity -
 * sizeof(ControlArena)`.
 *
 * @param backing_buffer Pointer to a allocated chunk of memory.
 * @param capacity Total size of the backing_buffer, in bytes.
 * @return Pointer to an initialized ControlArena, or NULL if capacity is to small.
 */
ControlArena *Control_Arena_Create(void *backing_buffer, size_t capacity);

/**
 * @brief Resets arena's allocation offset to zero.
 *
 * This invalidates all pointers previously in the arena, while not
 * zeroing out the backing buffer, ensuring a O(1) operation.
 *
 * @warning Pointers allocated in the arena are not guranteed to be NULL.
 * Using the pointers allocated in the arena is **undefined behavior**.
 *
 * @param arena Pointer to the arena to clear.
 */
void Control_Arena_Clear(ControlArena *arena);

/**
 * @brief Calculates the remaining bytes in the arena.
 *
 * @param Pointer to the arena.
 * @return Number of possible bytes available for allocation
 */
size_t Control_Arena_RemainingSpace(ControlArena *arena);

/**
 * @brief Allocates a block of memory in the arena.
 *
 * @param arena Pointer to the arena.
 * @param size Number of bytes to allocate
 *
 * @return Pointer to newly allocated memory, or NULL if size is 0 or if the arena did not have
 * enough capacity.
 */
void *Control_Arena_Alloc(ControlArena *arena, size_t size);
#endif
