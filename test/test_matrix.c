#include <controlly/arena.h>
#include <controlly/core.h>
#include <controlly/matrix.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unity.h>
#include <unity_fixture.h>

TEST_GROUP(Matrix);

static void         *p_pool;
static void         *s_pool;
static ControlHandle ctx;
static ControlResult last_error_code = CONTROL_OK;

static void MockErrorHandler(ControlResult code, const char *msg, void *user_data)
{
    last_error_code = code;
}

TEST_SETUP(Matrix)
{
    const static size_t MEMSIZE = 1024;
    p_pool                      = malloc(sizeof(uint8_t) * MEMSIZE);
    s_pool                      = malloc(sizeof(uint8_t) * MEMSIZE);

    ControlArena *p = Control_Arena_Create(p_pool, MEMSIZE);
    ControlArena *s = Control_Arena_Create(s_pool, MEMSIZE);
    Control_System_Init(&ctx, p, s);

    ctx.on_error = MockErrorHandler;
}

TEST_TEAR_DOWN(Matrix)
{
    Control_System_DeInit(&ctx);
    free(p_pool);
    free(s_pool);
}

TEST(Matrix, CanDeepCloneAVector)
{
    float      coeffs[] = {1.0f, 2.0f, 3.0f, 4.0f};
    ControlVec v        = {
               .size     = 4,
               .capacity = 4,
               .coeffs   = coeffs,
    };

    ControlVec v2 = {0};
    TEST_ASSERT_EQUAL(CONTROL_OK, Control_Vec_Persist(&ctx, &v2, &v));
    TEST_ASSERT_TRUE(Control_Vec_IsValid(&v2));
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(coeffs, v2.coeffs, 4);
    TEST_ASSERT_TRUE(v.coeffs != v2.coeffs);
}

TEST(Matrix, CanDeepCloneAMatrix)
{
    // 2x2 matrix
    float         coeffs[] = {1.0f, 2.0f, 3.0f, 4.0f};
    ControlMatrix m        = {
               .rows = 2,
               .cols = 2,
               .data = coeffs,
    };

    ControlMatrix m2 = {0};
    TEST_ASSERT_EQUAL(CONTROL_OK, Control_Matrix_Persist(&ctx, &m2, &m));
    TEST_ASSERT_TRUE(Control_Matrix_IsValid(&m2));
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(coeffs, m2.data, 4);
    TEST_ASSERT_TRUE(m.data != m2.data);
}

TEST_GROUP_RUNNER(Matrix)
{
    RUN_TEST_CASE(Matrix, CanDeepCloneAVector);
    RUN_TEST_CASE(Matrix, CanDeepCloneAMatrix);
}
