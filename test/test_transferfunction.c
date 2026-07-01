#include <ccontrol/tf.h>
#include <stdlib.h>
#include <unity.h>
#include <unity_fixture.h>

TEST_GROUP(TransferFunction);

static void *p_pool;
static void *s_pool;
static ControlHandle ctx;

TEST_SETUP(TransferFunction)
{
    const static size_t MEMSIZE = 1024;
    p_pool = malloc(sizeof(uint8_t) * MEMSIZE);
    s_pool = malloc(sizeof(uint8_t) * MEMSIZE);

    ControlArena *p = ControlArena_Create(p_pool, MEMSIZE);
    ControlArena *s = ControlArena_Create(s_pool, MEMSIZE);
    ControlSystem_InitHandle(&ctx, p, s);
}

TEST_TEAR_DOWN(TransferFunction)
{
    ControlSystem_DeInitHandle(&ctx);
    free(p_pool);
    free(s_pool);
}

TEST(TransferFunction, CanCreateTransferFunctionFromCoefficients)
{
    // H(s) = (2s + 3) / (s^2 + 4s + 5)
    float n_val[] = {2.0f, 3.0f};
    float d_val[] = {1.0f, 4.0f, 5.0f};

    control_vector_t num = PolyCoeffVector_Persistent(&ctx, n_val, 2);
    control_vector_t dem = PolyCoeffVector_Persistent(&ctx, d_val, 3);
    TransferFunction tf = TransferFunctionFromCoeffs(&num, &dem);

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
    control_vector_t num1 = PolyCoeffVector_Persistent(&ctx, n1, 1);
    control_vector_t dem1 = PolyCoeffVector_Persistent(&ctx, d1, 2);
    TransferFunction G1 = TransferFunctionFromCoeffs(&num1, &dem1);

    // G2(s) = 2 / (s + 3)
    float n2[] = {2.0f};
    float d2[] = {1.0f, 3.0f};
    control_vector_t num2 = PolyCoeffVector_Persistent(&ctx, n2, 1);
    control_vector_t dem2 = PolyCoeffVector_Persistent(&ctx, d2, 2);
    TransferFunction G2 = TransferFunctionFromCoeffs(&num2, &dem2);

    // Act: G_sys = G1 * G2
    TransferFunction G_sys = MultiplyTransferFunctions(&ctx, &G1, &G2);

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
    control_vector_t num = PolyCoeffVector_Persistent(&ctx, n, 1);
    control_vector_t dem = PolyCoeffVector_Persistent(&ctx, d, 2);
    TransferFunction G = TransferFunctionFromCoeffs(&num, &dem);

    // Act: Closed loop T(s) = G(s) / (1 + G(s))
    TransferFunction T =
        UnityClosedLoop(&ctx, &G, 1.0f, 0); // Gain/Unity enum mocked as 1.0f, 0

    // Expected: 10 / (s + 12)
    float expected_dem[] = {1.0f, 12.0f};

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(n, T.num.coeffs, 1);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected_dem, T.dem.coeffs, 2);
}

TEST_GROUP_RUNNER(TransferFunction)
{
    RUN_TEST_CASE(TransferFunction, CanCreateTransferFunctionFromCoefficients);
    RUN_TEST_CASE(TransferFunction, MultiplyTwoTransferFunctions);
    RUN_TEST_CASE(TransferFunction, UnityClosedLoopReduction);
}
