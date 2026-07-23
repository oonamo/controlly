#include <ccontrol/tf.h>
#include <stdio.h>
#include <stdlib.h>
#include <unity.h>
#include <unity_fixture.h>

#include <ccontrol/statespace.h>

TEST_GROUP(StateSpace);

static void *p_pool;
static void *s_pool;
static ControlHandle ctx;

TEST_SETUP(StateSpace)
{
    const static size_t MEMSIZE = 1024;
    p_pool = malloc(sizeof(uint8_t) * MEMSIZE);
    s_pool = malloc(sizeof(uint8_t) * MEMSIZE);

    ControlArena *p = Control_Arena_Create(p_pool, MEMSIZE);
    ControlArena *s = Control_Arena_Create(s_pool, MEMSIZE);
    Control_System_Init(&ctx, p, s);
}

TEST_TEAR_DOWN(StateSpace)
{
    Control_System_DeInit(&ctx);
    free(p_pool);
    free(s_pool);
}

TEST(StateSpace, CanConvertTransferFunction)
{
    float n[] = {10.0f};
    float d[] = {1.0f, 2.0f, 5.0f};

    ControlVec num = {0}, dem = {0};
    ControlTransferFunction tf = {0};
    ControlStateSpace sys = {0};

    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_Poly_AllocPersistent(&ctx, &num, n, 1));
    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_Poly_AllocPersistent(&ctx, &dem, d, 3));
    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_TF_FromPoly(&ctx, &tf, &num, &dem));

    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_StateSpace_FromTF(&ctx, &sys, &tf));

    // 2x2 matrix
    float expected_A[] = {0.0f, 1.0f, -5.0f, -2.0f};
    // 2x1
    float expected_B[] = {0.0f, 1.0f};
    // 1x2
    float expected_C[] = {10.0f, 0.0f};
    // 1x1
    float expected_D[] = {0.0f};

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected_A, sys.A.data, 4);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected_B, sys.B.data, 2);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected_C, sys.C.data, 2);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected_D, sys.D.data, 1);
}

TEST(StateSpace, ContinousSISORealizationWithStepResponse)
{
    float n[] = {10.0f};
    float d[] = {1.0f, 2.0f, 5.0f};

    ControlVec num = {0}, dem = {0};
    ControlTransferFunction tf = {0};
    ControlStateSpace sys = {0};

    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_Poly_AllocPersistent(&ctx, &num, n, 1));
    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_Poly_AllocPersistent(&ctx, &dem, d, 3));
    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_TF_FromPoly(&ctx, &tf, &num, &dem));

    TEST_ASSERT_EQUAL(CCONTROL_OK, Control_StateSpace_FromTF(&ctx, &sys, &tf));

    float x_data[] = {0.0f, 0.0f}; // Initial states at 0
    float u_data[] = {1.0f};       // Constant step input of 1.0
    float y_data[] = {0.0f};       // Output buffer

    // FIX: Added `.coeffs =` for proper designated initialization
    sys.x = (ControlVec){.size = 2, .capacity = 2, .coeffs = x_data};
    sys.u = (ControlVec){.size = 1, .capacity = 1, .coeffs = u_data};
    sys.y = (ControlVec){.size = 1, .capacity = 1, .coeffs = y_data};

    float Ts = 0.1f;
    for (int k = 0; k < 3; k++)
    {
        Control_StateSpace_StepContinuous(&ctx, &sys, Ts);
    }

    // NOTE: The y output is calculated at the BEGINNING of the 3rd tick (k=2).
    // y = (10 * 0.01) + (0 * 0.18) = 0.100f.
    float expected_x[] = {0.028f, 0.239f};
    float expected_y[] = {0.100f};

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected_x, sys.x.coeffs, 2);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected_y, sys.y.coeffs, 1);
}

TEST(StateSpace, ContinousMIMORealizationWithStepResponse)
{
    ControlStateSpace sys = {0};
    float a_data[] = {-1.0f, 1.0f, -1.0f, -2.0f};
    float b_data[] = {1.0f, 2.0f, 3.0f, 4.0f};
    float c_data[] = {5.0f, 6.0f, 7.0f, 8.0f};
    float d_data[] = {1.0f, 0.0f, 0.0f, 1.0f};

    float x_data[] = {0.0f, 0.0f}; // 2 states
    float u_data[] = {1.0f, 1.0f}; // 2 constant step inputs
    float y_data[] = {0.0f, 0.0f}; // 2 outputs

    // FIX: Corrected `.size` and `.capacity` to 2 for the u and y MIMO vectors.
    sys.x = (ControlVec){.size = 2, .capacity = 2, .coeffs = x_data};
    sys.u = (ControlVec){.size = 2, .capacity = 2, .coeffs = u_data};
    sys.y = (ControlVec){.size = 2, .capacity = 2, .coeffs = y_data};

    // FIX: Removed the trailing 'j' from ControlSystemMatrixj
    sys.A = (ControlSystemMatrix){.data = a_data, .rows = 2, .cols = 2};
    sys.B = (ControlInputMatrix){.data = b_data, .rows = 2, .cols = 2};
    sys.C = (ControlOutputMatrix){.data = c_data, .rows = 2, .cols = 2};
    sys.D = (ControlFeedbackMatrix){.data = d_data, .rows = 2, .cols = 2};

    float dt = 0.1f;
    for (int k = 0; k < 3; k++)
    {
        Control_StateSpace_StepContinuous(&ctx, &sys, dt);
    }

    // NOTE: The y output is calculated at the BEGINNING of the 3rd tick (k=2).
    float expected_x[] = {0.999f, 1.620f};
    float expected_y[] = {11.580f, 15.320f};

    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected_x, sys.x.coeffs, 2);
    TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected_y, sys.y.coeffs, 2);
}

TEST_GROUP_RUNNER(StateSpace)
{
    RUN_TEST_CASE(StateSpace, CanConvertTransferFunction);
    RUN_TEST_CASE(StateSpace, ContinousSISORealizationWithStepResponse);
    RUN_TEST_CASE(StateSpace, ContinousMIMORealizationWithStepResponse);
}
