#include "ccontrol/arena.h"
#include <ccontrol/tf.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unity.h>
#include <unity_fixture.h>

TEST_GROUP(PolyMath);

static void *p_pool;
static void *s_pool;
static ControlHandle ctx;

TEST_SETUP(PolyMath)
{
    const static size_t MEMSIZE = 1024;
    p_pool = malloc(sizeof(uint8_t) * MEMSIZE);
    s_pool = malloc(sizeof(uint8_t) * MEMSIZE);

    ControlArena *p = Control_Arena_Create(p_pool, MEMSIZE);
    ControlArena *s = Control_Arena_Create(s_pool, MEMSIZE);
    Control_System_Init(&ctx, p, s);
}

TEST_TEAR_DOWN(PolyMath)
{
    Control_System_DeInit(&ctx);
    free(p_pool);
    free(s_pool);
}

TEST(PolyMath, PolynomialMultiplication)
{
    // s^2 + 2s + 3
    float a[3] = {1, 2, 3};
    // s + 2
    float b[2] = {1, 2};

    ControlVec a_coeffs = {0}, b_coeffs = {0};
    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_Poly_AllocScratch(&ctx, &a_coeffs, a, 3));
    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_Poly_AllocScratch(&ctx, &b_coeffs, b, 2));

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(a, a_coeffs.coeffs, 3);
    TEST_ASSERT_EQUAL_size_t(3, a_coeffs.size);

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(b, b_coeffs.coeffs, 2);
    TEST_ASSERT_EQUAL_size_t(2, b_coeffs.size);

    // s^3 + 4s^2 + 7s + 6
    ControlVec result = {0};
    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_Poly_Multiply(&ctx, &result, &a_coeffs, &b_coeffs));

    float expected_result[] = {1, 4, 7, 6};

    TEST_ASSERT_EQUAL_size_t(4, result.size);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected_result, result.coeffs, 4);
}

TEST(PolyMath, PolynomialAdditionSameSize)
{
    float a[3] = {1.0f, 2.0f, 3.0f};
    float b[3] = {4.0f, 5.0f, 6.0f};

    ControlVec a_coeffs = {0}, b_coeffs = {0}, result = {0};
    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_Poly_AllocScratch(&ctx, &a_coeffs, a, 3));
    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_Poly_AllocScratch(&ctx, &b_coeffs, b, 3));

    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_Poly_Add(&ctx, &result, &a_coeffs, &b_coeffs));

    float expected[] = {5.0f, 7.0f, 9.0f};
    TEST_ASSERT_EQUAL_size_t(3, result.size);
    TEST_ASSERT_FLOAT_ARRAY_WITHIN(0.001f, expected, result.coeffs, result.size);
}

TEST(PolyMath, PolynomialAdditionDifferentSize)
{
    // Adding s^2 + 2s + 3 and 4s + 5
    float a[3] = {1.0f, 2.0f, 3.0f};
    float b[2] = {4.0f, 5.0f};

    ControlVec a_coeffs = {0}, b_coeffs = {0}, result = {0};
    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_Poly_AllocScratch(&ctx, &a_coeffs, a, 3));
    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_Poly_AllocScratch(&ctx, &b_coeffs, b, 2));

    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_Poly_Add(&ctx, &result, &a_coeffs, &b_coeffs));

    float expected[] = {1.0f, 6.0f, 8.0f};

    TEST_ASSERT_EQUAL_size_t(3, result.size);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected, result.coeffs, 3);
}

TEST(PolyMath, RepeatedMathDoesNotOOM)
{
    float a[3] = {1.0f, 2.0f, 3.0f};
    float b[2] = {4.0f, 5.0f};

    // FIX: These must be in the PERSISTENT arena for this test loop!
    ControlVec a_coeffs = {0}, b_coeffs = {0};
    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_Poly_AllocPersistent(&ctx, &a_coeffs, a, 3));
    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_Poly_AllocPersistent(&ctx, &b_coeffs, b, 2));

    ControlVec temp = {0};

    // Loop 1000 times to ensure scratch wiping works
    for (int i = 0; i < 1000; i++)
    {
        // Because we wipe the arena at the end of the loop, temp's pointers become invalid.
        // We must reset temp to 0 so the math function knows to re-allocate it.
        temp = (ControlVec){0};

        TEST_ASSERT_EQUAL(CCONTROL_OK, Control_Poly_Multiply(&ctx, &temp, &a_coeffs, &b_coeffs));
        Control_Arena_Clear(ctx.scratch);
    }

    TEST_ASSERT_MESSAGE(true, "Survived 1000 loops without OOM");
}

TEST(PolyMath, CanonicalizeRemovesLeadingZeroes)
{
    float c[] = {0.0f, 0.0f, 1.0f, 0.0f, 2.0f};
    float expected[] = {1.0f, 0.0f, 2.0f};

    ControlVec uncanon_vec = {0}, canon_vec = {0};
    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_Poly_AllocScratch(&ctx, &uncanon_vec, c, 5));

    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_Poly_Canonicalize(&ctx, &canon_vec, &uncanon_vec));

    TEST_ASSERT_EQUAL_size_t(3, canon_vec.size);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected, canon_vec.coeffs, 3);
}

TEST_GROUP_RUNNER(PolyMath)
{
    RUN_TEST_CASE(PolyMath, PolynomialMultiplication);
    RUN_TEST_CASE(PolyMath, PolynomialAdditionSameSize);
    RUN_TEST_CASE(PolyMath, PolynomialAdditionDifferentSize);
    RUN_TEST_CASE(PolyMath, RepeatedMathDoesNotOOM);
    RUN_TEST_CASE(PolyMath, CanonicalizeRemovesLeadingZeroes);
}
