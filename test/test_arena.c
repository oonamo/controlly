#include "unity.h"
#include <ccontrol/arena.h>
#include <string.h>
#include <unity_fixture.h>

TEST_GROUP(Arena);

// TODO: Test this on embedded targets
#if defined(__GNUC__) || defined(__clang__)
#    define CCONTROL_ALIGN(x) __attribute__((aligned(x)))
#elif defined(_MSC_VER)
#    define CCONTROL_ALIGN(x) __declspec(align(x))
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
// Fallback to C11 _Alignas if it's a modern, non-GCC/MSVC compiler
#    define CCONTROL_ALIGN(x) _Alignas(x)
#else
#    error "Compiler not supported for memory alignment."
#endif

#define STATIC_SIZE  2048
#define DYNAMIC_SIZE 2048

CCONTROL_ALIGN(CCONTROL_ARENA_ALIGN_SIZE)
static uint8_t static_mempool[STATIC_SIZE] = {0};
ControlArena *arena;

static size_t expected_capacity;

TEST_SETUP(Arena)
{
    arena = Control_Arena_Create(static_mempool, STATIC_SIZE);
    expected_capacity = STATIC_SIZE - sizeof(ControlArena);
}

TEST_TEAR_DOWN(Arena)
{
    arena = NULL;
}

TEST(Arena, InitialCreationTracksCapacityCorrectly)
{
    TEST_ASSERT_EQUAL_size_t(expected_capacity, Control_Arena_RemainingSpace(arena));
}

TEST(Arena, AllocationReducesRemainingSpace)
{
    size_t alloc_size = 64;
    void *ptr = Control_Arena_Alloc(arena, alloc_size);

    TEST_ASSERT_NOT_NULL(ptr);
    TEST_ASSERT_EQUAL_size_t(expected_capacity - alloc_size, Control_Arena_RemainingSpace(arena));
}

TEST(Arena, SequentialAllocsDoNotOverlap)
{
    void *ptr1 = Control_Arena_Alloc(arena, 10);
    void *ptr2 = Control_Arena_Alloc(arena, 10);

    TEST_ASSERT_NOT_NULL(ptr1);
    TEST_ASSERT_NOT_NULL(ptr2);
    TEST_ASSERT_NOT_EQUAL_PTR(ptr1, ptr2);
}

TEST(Arena, AllocatingExactCapacityWorks)
{
    void *ptr = Control_Arena_Alloc(arena, Control_Arena_RemainingSpace(arena));
    TEST_ASSERT_NOT_NULL(ptr);
    TEST_ASSERT_EQUAL_size_t(0, Control_Arena_RemainingSpace(arena));
}

TEST(Arena, AllocReturnsNullWhenExhausted)
{
    void *ptr1 = Control_Arena_Alloc(arena, expected_capacity - CCONTROL_ARENA_ALIGN_SIZE);
    void *ptr2 = Control_Arena_Alloc(arena, CCONTROL_ARENA_ALIGN_SIZE + 1);

    TEST_ASSERT_NOT_NULL(ptr1);
    TEST_ASSERT_NULL(ptr2);
    TEST_ASSERT_EQUAL_size_t(CCONTROL_ARENA_ALIGN_SIZE, Control_Arena_RemainingSpace(arena));
}

TEST(Arena, ClearResetsCapacityAndReusesMemory)
{
    void *alloc1 = Control_Arena_Alloc(arena, expected_capacity / 2);
    TEST_ASSERT_NOT_NULL(alloc1);

    Control_Arena_Clear(arena);
    TEST_ASSERT_NOT_NULL(alloc1);

    TEST_ASSERT_EQUAL_size_t(expected_capacity, Control_Arena_RemainingSpace(arena));

    void *alloc2 = Control_Arena_Alloc(arena, expected_capacity / 2);

    TEST_ASSERT_EQUAL_PTR(alloc1, alloc2);
}

TEST(Arena, HandlesAllocOfZero)
{
    size_t initial_space = Control_Arena_RemainingSpace(arena);
    void *ptr = Control_Arena_Alloc(arena, 0);

    TEST_ASSERT_NULL(ptr);
    TEST_ASSERT_EQUAL_size_t(initial_space, Control_Arena_RemainingSpace(arena));
}

TEST(Arena, CreationFailsIfCapacityIsTooSmall)
{
    ControlArena *new_arena = Control_Arena_Create(static_mempool, sizeof(ControlArena) - 1);
    TEST_ASSERT_NULL(new_arena);
}

TEST_GROUP_RUNNER(Arena)
{
    RUN_TEST_CASE(Arena, InitialCreationTracksCapacityCorrectly);
    RUN_TEST_CASE(Arena, AllocationReducesRemainingSpace);
    RUN_TEST_CASE(Arena, SequentialAllocsDoNotOverlap);
    RUN_TEST_CASE(Arena, AllocatingExactCapacityWorks);
    RUN_TEST_CASE(Arena, AllocReturnsNullWhenExhausted);
    RUN_TEST_CASE(Arena, ClearResetsCapacityAndReusesMemory);
    RUN_TEST_CASE(Arena, HandlesAllocOfZero);
    RUN_TEST_CASE(Arena, HandlesAllocOfZero);
    RUN_TEST_CASE(Arena, CreationFailsIfCapacityIsTooSmall);
}
