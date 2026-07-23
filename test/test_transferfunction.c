#include "ccontrol/arena.h"
#include <ccontrol/core.h>
#include <ccontrol/tf.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>
#include <unity_fixture.h>

TEST_GROUP(TransferFunction);

static void *p_pool;
static void *s_pool;
static ControlHandle ctx;
static ControlResult last_error_code = CCONTROL_OK;

static void MockErrorHandler(ControlResult code, const char *msg, void *user_data)
{
    last_error_code = code;
}

TEST_SETUP(TransferFunction)
{
    const static size_t MEMSIZE = 1024;
    p_pool = malloc(sizeof(uint8_t) * MEMSIZE);
    s_pool = malloc(sizeof(uint8_t) * MEMSIZE);

    ControlArena *p = Control_Arena_Create(p_pool, MEMSIZE);
    ControlArena *s = Control_Arena_Create(s_pool, MEMSIZE);
    Control_System_Init(&ctx, p, s);

    last_error_code = CCONTROL_OK;
    ctx.on_error = MockErrorHandler;
}

TEST_TEAR_DOWN(TransferFunction)
{
    Control_System_DeInit(&ctx);
    free(p_pool);
    free(s_pool);
}

TEST(TransferFunction, CanCreateTransferFunctionFromCoefficients)
{
    // H(s) = (2s + 3) / (s^2 + 4s + 5)
    float n_val[] = {2.0f, 3.0f};
    float d_val[] = {1.0f, 4.0f, 5.0f};

    ControlVec num = {0};
    ControlVec dem = {0};
    ControlTransferFunction tf = {0};

    // Assert that the allocations succeed
    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_Poly_AllocPersistent(&ctx, &num, n_val, 2));
    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_Poly_AllocPersistent(&ctx, &dem, d_val, 3));

    // Assert that the TF construction succeeds
    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_TF_FromPoly(&ctx, &tf, &num, &dem));

    TEST_ASSERT_EQUAL_size_t(2, tf.num.size);
    TEST_ASSERT_EQUAL_size_t(3, tf.den.size);

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(n_val, tf.num.coeffs, 2);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(d_val, tf.den.coeffs, 3);
}

TEST(TransferFunction, MultiplyTwoTransferFunctions)
{
    // G1(s) = 1 / (s + 2)
    float n1[] = {1.0f};
    float d1[] = {1.0f, 2.0f};
    ControlVec num1 = {0}, dem1 = {0};
    ControlTransferFunction G1 = {0};
    Control_Poly_AllocPersistent(&ctx, &num1, n1, 1);
    Control_Poly_AllocPersistent(&ctx, &dem1, d1, 2);
    Control_TF_FromPoly(&ctx, &G1, &num1, &dem1);

    // G2(s) = 2 / (s + 3)
    float n2[] = {2.0f};
    float d2[] = {1.0f, 3.0f};
    ControlVec num2 = {0}, dem2 = {0};
    ControlTransferFunction G2 = {0};
    Control_Poly_AllocPersistent(&ctx, &num2, n2, 1);
    Control_Poly_AllocPersistent(&ctx, &dem2, d2, 2);
    Control_TF_FromPoly(&ctx, &G2, &num2, &dem2);

    // Act: G_sys = G1 * G2
    ControlTransferFunction G_sys = {0};
    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_TF_Multiply(&ctx, &G_sys, &G1, &G2));

    // Expected: 2 / (s^2 + 5s + 6)
    float expected_num[] = {2.0f};
    float expected_dem[] = {1.0f, 5.0f, 6.0f};

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected_num, G_sys.num.coeffs, 1);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected_dem, G_sys.den.coeffs, 3);
}

TEST(TransferFunction, UnityClosedLoopReduction)
{
    // Forward path G(s) = 10 / (s + 2)
    float n[] = {10.0f};
    float d[] = {1.0f, 2.0f};
    ControlVec num = {0}, dem = {0};
    ControlTransferFunction G = {0};

    Control_Poly_AllocPersistent(&ctx, &num, n, 1);
    Control_Poly_AllocPersistent(&ctx, &dem, d, 2);
    Control_TF_FromPoly(&ctx, &G, &num, &dem);

    // Act: Closed loop T(s) = G(s) / (1 + G(s))
    ControlTransferFunction T = {0};
    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_TF_ClosedLoop(&ctx, &T, &G, 1.0f, TF_FEEDBACK_NEGATIVE));

    // Expected: 10 / (s + 12)
    float expected_dem[] = {1.0f, 12.0f};

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(n, T.num.coeffs, 1);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected_dem, T.den.coeffs, 2);
}

TEST(TransferFunction, ValidateDetectsEmptyTransferFunction)
{
    ControlTransferFunction empty = CCONTROL_EMPTY_TF;

    TEST_ASSERT_EQUAL(CCONTROL_ERROR_INVALID_ARGUMENT, Control_TF_Validate(&ctx, &empty));
    TEST_ASSERT_EQUAL(CCONTROL_ERROR_INVALID_ARGUMENT, last_error_code);
}

TEST(TransferFunction, MultiplyInvalidTransferFunctionErrorsInvalidArgument)
{
    float n[] = {1.0f};
    float d[] = {1.0f};
    ControlVec num = {0}, dem = {0};
    ControlTransferFunction G1 = {0};

    Control_Poly_AllocPersistent(&ctx, &num, n, 1);
    Control_Poly_AllocPersistent(&ctx, &dem, d, 1);
    Control_TF_FromPoly(&ctx, &G1, &num, &dem);

    ControlTransferFunction G2 = CCONTROL_EMPTY_TF;
    ControlTransferFunction result = {0};

    ControlResult status = Control_TF_Multiply(&ctx, &result, &G1, &G2);

    TEST_ASSERT_EQUAL(CCONTROL_ERROR_INVALID_ARGUMENT, Control_TF_Validate(&ctx, &result));
    TEST_ASSERT_EQUAL(CCONTROL_ERROR_INVALID_ARGUMENT, status);
    TEST_ASSERT_EQUAL(CCONTROL_ERROR_INVALID_ARGUMENT, last_error_code);
}

TEST(TransferFunction, MultiplyOutOfMemorySafelyAbortsAndThrows)
{
    float n[] = {1.0f};
    float d[] = {1.0f};
    ControlVec num = {0}, dem = {0};
    ControlTransferFunction G1 = {0}, G2 = {0};

    Control_Poly_AllocPersistent(&ctx, &num, n, 1);
    Control_Poly_AllocPersistent(&ctx, &dem, d, 1);
    Control_TF_FromPoly(&ctx, &G1, &num, &dem);
    Control_TF_FromPoly(&ctx, &G2, &num, &dem);

    // Exhaust the scratch arena to force OOM during temporary math allocations
    size_t space_left = Control_Arena_RemainingSpace(ctx.scratch);
    Control_Arena_Alloc(ctx.scratch, space_left);

    ControlTransferFunction result = {0};
    ControlResult status = Control_TF_Multiply(&ctx, &result, &G1, &G2);

    TEST_ASSERT_EQUAL(CCONTROL_ERROR_OUT_OF_MEMORY, status);
    TEST_ASSERT_EQUAL(CCONTROL_ERROR_OUT_OF_MEMORY, last_error_code);
    TEST_ASSERT_EQUAL(CCONTROL_ERROR_INVALID_ARGUMENT, Control_TF_Validate(&ctx, &result));
    TEST_ASSERT_EQUAL(CCONTROL_ERROR_INVALID_ARGUMENT, last_error_code);
}

TEST(TransferFunction, CanDeepCloneTransferFunction)
{
    float tf_n[] = {1.0f, 2.0f, 3.0f};
    float tf_d[] = {1.0f, 2.0f, 3.0f, 8.0f};

    ControlVec num = {0}, dem = {0};
    ControlTransferFunction G1 = {0};

    Control_Poly_AllocScratch(&ctx, &num, tf_n, 3);
    Control_Poly_AllocScratch(&ctx, &dem, tf_d, 4);
    Control_TF_FromPoly(&ctx, &G1, &num, &dem);

    ControlTransferFunction cloned_tf = {0};
    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_TF_Persist(&ctx, &cloned_tf, &G1));

    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_TF_Validate(&ctx, &cloned_tf));
    TEST_ASSERT_TRUE(G1.num.coeffs != cloned_tf.num.coeffs &&
                     G1.den.coeffs != cloned_tf.den.coeffs);

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(tf_n, cloned_tf.num.coeffs, 3);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(tf_d, cloned_tf.den.coeffs, 4);

    // Clear scratch arena
    Control_Arena_Clear(ctx.scratch);

    // Fill scratch arena to check that TF was cloned to persistent arena
    size_t remaining_space = Control_Arena_RemainingSpace(ctx.scratch);
    void *mem = Control_Arena_Alloc(ctx.scratch, remaining_space);
    TEST_ASSERT_NOT_NULL(mem);

    memset(mem, 0xFF, remaining_space);

    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_TF_Validate(&ctx, &cloned_tf));
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(tf_n, cloned_tf.num.coeffs, 3);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(tf_d, cloned_tf.den.coeffs, 4);
}

TEST_GROUP_RUNNER(TransferFunction)
{
    RUN_TEST_CASE(TransferFunction, CanCreateTransferFunctionFromCoefficients);
    RUN_TEST_CASE(TransferFunction, MultiplyTwoTransferFunctions);
    RUN_TEST_CASE(TransferFunction, UnityClosedLoopReduction);
    RUN_TEST_CASE(TransferFunction, ValidateDetectsEmptyTransferFunction);
    RUN_TEST_CASE(TransferFunction, MultiplyInvalidTransferFunctionErrorsInvalidArgument);
    RUN_TEST_CASE(TransferFunction, MultiplyOutOfMemorySafelyAbortsAndThrows); // Added!
    RUN_TEST_CASE(TransferFunction, CanDeepCloneTransferFunction);
}
