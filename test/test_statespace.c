#include <ccontrol/tf.h>
#include <stdlib.h>
#include <unity.h>
#include <unity_fixture.h>

TEST_GROUP(StateSpace);

static void *p_pool;
static void *s_pool;
static ControlHandle ctx;

TEST_SETUP(StateSpace)
{
    const static size_t MEMSIZE = 1024;
    p_pool = malloc(sizeof(uint8_t) * MEMSIZE);
    s_pool = malloc(sizeof(uint8_t) * MEMSIZE);

    ControlArena *p = ControlArena_Create(p_pool, MEMSIZE);
    ControlArena *s = ControlArena_Create(s_pool, MEMSIZE);
    ControlSystem_InitHandle(&ctx, p, s);
}

TEST_TEAR_DOWN(StateSpace)
{
    ControlSystem_DeInitHandle(&ctx);
    free(p_pool);
    free(s_pool);
}

TEST(StateSpace, CanConvertTransferFunction)
{
    float n[] = {10.0f};
    float d[] = {1.0f, 2.0f, 5.0f};

    control_vector_t num = PolyCoeffVector_Persistent(&ctx, n, 1);
    control_vector_t dem = PolyCoeffVector_Persistent(&ctx, d, 3);
    TransferFunction tf = TransferFunctionFromCoeffs(&num, &dem);

    StateSpace sys = TransferFunctionToStateSpace(&ctx, &tf);

    // 2x2 matrix
    float expected_A[] = {0.0f, 1.0f, -5.0f, -2.0f};
    // 2x1
    float expected_B[] = {0.0f, 1.0f};
    // 1x2
    float expected_C[] = {10, 0};
    // 1x1
    float expected_D[] = {0.0f};

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected_A, sys.A.data, 4);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected_B, sys.B.coeffs, 2);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected_C, sys.C.coeffs, 2);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected_D, sys.D.coeffs, 1);
}

TEST_GROUP_RUNNER(StateSpace)
{
    RUN_TEST_CASE(StateSpace, CanConvertTransferFunction);
}
