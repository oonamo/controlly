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

    ControlVec num = Control_Poly_AllocPersistent(&ctx, n_val, 2);
    ControlVec dem = Control_Poly_AllocPersistent(&ctx, d_val, 3);
    ControlTransferFunction tf = Control_TF_FromPoly(&num, &dem);

    TEST_ASSERT_EQUAL_size_t(2, tf.num.size);
    TEST_ASSERT_EQUAL_size_t(3, tf.dem.size);

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(n_val, tf.num.coeffs, 2);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(d_val, tf.dem.coeffs, 3);
}

TEST(TransferFunction, MultiplyTwoTransferFunctions)
{
    // G1(s) = 1 / (s + 2)
    float n1[] = {1.0f};
    float d1[] = {1.0f, 2.0f};
    ControlVec num1 = Control_Poly_AllocPersistent(&ctx, n1, 1);
    ControlVec dem1 = Control_Poly_AllocPersistent(&ctx, d1, 2);
    ControlTransferFunction G1 = Control_TF_FromPoly(&num1, &dem1);

    // G2(s) = 2 / (s + 3)
    float n2[] = {2.0f};
    float d2[] = {1.0f, 3.0f};
    ControlVec num2 = Control_Poly_AllocPersistent(&ctx, n2, 1);
    ControlVec dem2 = Control_Poly_AllocPersistent(&ctx, d2, 2);
    ControlTransferFunction G2 = Control_TF_FromPoly(&num2, &dem2);

    // Act: G_sys = G1 * G2
    ControlTransferFunction G_sys = Control_TF_Multiply(&ctx, &G1, &G2);

    // Expected: 2 / (s^2 + 5s + 6)
    float expected_num[] = {2.0f};
    float expected_dem[] = {1.0f, 5.0f, 6.0f};

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected_num, G_sys.num.coeffs, 1);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected_dem, G_sys.dem.coeffs, 3);
}

TEST(TransferFunction, UnityClosedLoopReduction)
{
    // Forward path G(s) = 10 / (s + 2)
    float n[] = {10.0f};
    float d[] = {1.0f, 2.0f};
    ControlVec num = Control_Poly_AllocPersistent(&ctx, n, 1);
    ControlVec dem = Control_Poly_AllocPersistent(&ctx, d, 2);
    ControlTransferFunction G = Control_TF_FromPoly(&num, &dem);

    // Act: Closed loop T(s) = G(s) / (1 + G(s))
    ControlTransferFunction T =
        Control_TF_ClosedLoop(&ctx, &G, 1.0f, 0); // Gain/Unity enum mocked as 1.0f, 0

    // Expected: 10 / (s + 12)
    float expected_dem[] = {1.0f, 12.0f};

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(n, T.num.coeffs, 1);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected_dem, T.dem.coeffs, 2);
}

TEST(TransferFunction, IsValidDetectsEmptyTransferFunction)
{
    ControlTransferFunction empty = CCONTROL_EMPTY_TF;
    TEST_ASSERT_FALSE(Control_TF_IsValid(&empty));
}

TEST(TransferFunction, MultiplyInvalidTransferFunctionErrorsInvalidArgument)
{
    float n[] = {1.0f};
    float d[] = {1.0f};
    ControlVec num = Control_Poly_AllocPersistent(&ctx, n, 1);
    ControlVec dem = Control_Poly_AllocPersistent(&ctx, d, 1);

    ControlTransferFunction G1 = Control_TF_FromPoly(&num, &dem);
    ControlTransferFunction G2 = CCONTROL_EMPTY_TF;

    ControlTransferFunction result = Control_TF_Multiply(&ctx, &G1, &G2);

    TEST_ASSERT_FALSE(Control_TF_IsValid(&result));
    TEST_ASSERT_EQUAL_INT(last_error_code, CCONTROL_ERROR_INVALID_ARGUMENT);
}

TEST(TransferFunction, MultiplyOutOfMemorySafelyAbortsAndThrows)
{
    float n[] = {1.0f};
    float d[] = {1.0f};
    ControlVec num = Control_Poly_AllocPersistent(&ctx, n, 1);
    ControlVec dem = Control_Poly_AllocPersistent(&ctx, d, 1);

    ControlTransferFunction G1 = Control_TF_FromPoly(&num, &dem);
    ControlTransferFunction G2 = Control_TF_FromPoly(&num, &dem);

    size_t space_left = Control_Arena_RemainingSpace(ctx.persistent);
    Control_Arena_Alloc(ctx.persistent, space_left);

    ControlTransferFunction result = Control_TF_Multiply(&ctx, &G1, &G2);
    TEST_ASSERT_FALSE(Control_TF_IsValid(&result));
    TEST_ASSERT_EQUAL_INT(last_error_code, CCONTROL_ERROR_OUT_OF_MEMORY);
}

TEST(TransferFunction, CanDeepCloneTransferFunction)
{
    float tf_n[] = {1.0f, 2.0f, 3.0f};
    float tf_d[] = {1.0f, 2.0f, 3.0f, 8.0f};

    ControlVec num = Control_Poly_AllocScratch(&ctx, tf_n, 3);
    ControlVec dem = Control_Poly_AllocScratch(&ctx, tf_d, 4);

    ControlTransferFunction G1 = Control_TF_FromPoly(&num, &dem);
    ControlTransferFunction cloned_tf = Control_TF_Persist(&ctx, &G1);

    TEST_ASSERT_TRUE(Control_TF_IsValid(&cloned_tf));
    TEST_ASSERT_TRUE(G1.num.coeffs != cloned_tf.num.coeffs &&
                     G1.dem.coeffs != cloned_tf.dem.coeffs);

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(tf_n, cloned_tf.num.coeffs, 3);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(tf_d, cloned_tf.dem.coeffs, 4);

    // Clear scratch arena
    Control_Arena_Clear(ctx.scratch);

    // Fill scratch arena to check that TF was cloned to persistent arena
    size_t remaining_space = Control_Arena_RemainingSpace(ctx.scratch);
    void *mem = Control_Arena_Alloc(ctx.scratch, remaining_space);
    TEST_ASSERT_NOT_NULL(mem);

    // TODO: Abstract memcpy to CCONTROL_MEMCPY or ctx->memcpy() (if needed?)
    // Also, is this safe for all testing environments
    memset(mem, 0, remaining_space);

    TEST_ASSERT_TRUE(Control_TF_IsValid(&cloned_tf));
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(tf_n, cloned_tf.num.coeffs, 3);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(tf_d, cloned_tf.dem.coeffs, 4);
}

TEST_GROUP_RUNNER(TransferFunction)
{
    RUN_TEST_CASE(TransferFunction, CanCreateTransferFunctionFromCoefficients);
    RUN_TEST_CASE(TransferFunction, MultiplyTwoTransferFunctions);
    RUN_TEST_CASE(TransferFunction, UnityClosedLoopReduction);
    RUN_TEST_CASE(TransferFunction, IsValidDetectsEmptyTransferFunction);
    RUN_TEST_CASE(TransferFunction, MultiplyInvalidTransferFunctionErrorsInvalidArgument);
    RUN_TEST_CASE(TransferFunction, CanDeepCloneTransferFunction);
}
