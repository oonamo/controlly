#include "ccontrol/arena.h"
#include <ccontrol/tf.h>
#include <stdint.h>
#include <stdlib.h>
#include <unity.h>
#include <unity_fixture.h>

TEST_GROUP(PolyMath);

static void *p_pool;
static void *s_pool;
static ControlHandle hndl;
ControlArena *p;
ControlArena *s;

TEST_SETUP(PolyMath)
{
    const static size_t MEMSIZE = 1024;
    p_pool = malloc(sizeof(uint8_t) * MEMSIZE);
    s_pool = malloc(sizeof(uint8_t) * MEMSIZE);

    ControlArena *p = ControlArena_Create(p_pool, MEMSIZE);
    ControlArena *s = ControlArena_Create(p_pool, MEMSIZE);
    ControlSystem_InitHandle(&hndl, p, s);
}

TEST_TEAR_DOWN(PolyMath)
{
    ControlSystemDeInit();
    free(p_pool);
    free(s_pool);
}

TEST(PolyMath, PolynomialMultiplication)
{
    // s^2 + 2s + 3
    float a[3] = {1, 2, 3};

    // s + 2
    float b[2] = {1, 2};

    control_vector_t a_coeffs = PolyCoeffVector(&hndl, a, 3);
    control_vector_t b_coeffs = PolyCoeffVector(&hndl, b, 2);

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(a, a_coeffs.coeffs, 3);
    TEST_ASSERT_EQUAL_size_t(3, a_coeffs.size);

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(b, b_coeffs.coeffs, 2);
    TEST_ASSERT_EQUAL_size_t(2, b_coeffs.size);

    // s^3 + 4s^2 + 7s + 6
    control_vector_t result = MultiplyPoly(&hndl, &a_coeffs, &b_coeffs);
    float expected_result[] = {1, 4, 7, 6};

    TEST_ASSERT_EQUAL_size_t(4, result.size);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected_result, result.coeffs, 4);
}

TEST(PolyMath, PolynomialAdditionSameSize)
{
    float a[3] = {1.0f, 2.0f, 3.0f};
    float b[3] = {4.0f, 5.0f, 6.0f};

    control_vector_t a_coeffs = PolyCoeffVector(&hndl, a, 3);
    control_vector_t b_coeffs = PolyCoeffVector(&hndl, b, 2);
}

TEST_GROUP_RUNNER(PolyMath)
{
    RUN_TEST_CASE(PolyMath, PolynomialMultiplication)
}
