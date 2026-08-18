#include <math.h>
#include <unity.h>
#include <unity_fixture.h>

#ifdef HEADERLIB_TEST
    #include "controlly.h"
#else
    #include <controlly/controllers/pid.h>
#endif

TEST_GROUP(PID);

static ControlPIDController pid;
static ControlPIDConfig     cfg;

TEST_SETUP(PID)
{
    cfg.enable_anti_windup = false;
    cfg.max_out            = 0.0f;
    cfg.min_out            = 0.0f;
}

TEST_TEAR_DOWN(PID)
{
    Control_PID_Reset(&pid);
}

TEST(PID, ProportionalTermOnly)
{
    Control_PID_Init(&pid, 5.0f, 0.0f, 0.0f, &cfg);

    // Error = 15 - 5 = 10;
    // Proportinal term should be: error * kp = 10 * 5.0 = 50;
    // Output = p term
    float output = Control_PID_Update(&pid, 15.0f, 5.0f, 0.1);

    TEST_ASSERT_EQUAL_FLOAT(50.0f, output);
}

TEST(PID, IntegralTermOnly)
{
    Control_PID_Init(&pid, 0.0f, 1.0f, 0.0f, &cfg);
    float dt = 0.1f;

    // Output:
    // error = 10.0f - 1.0f = 9.0f;
    // pid->integral = error * dt = 9.0 * 0.1 = 0.9;
    // i_term = kd * pid->integral = 1.0 * 0.9 = 0.9
    float out1 = Control_PID_Update(&pid, 10.0f, 1.0f, dt);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.9f, out1);

    // Output:
    // error = 10.0f - 1.0f = 9.0f;
    // pid->integral = pid->integral + error * dt = 0.9 +  9.0 * 0.1 = 1.8;
    // i_term = kd * pid->integral = 1.0 * 1.* = 1.8
    float out2 = Control_PID_Update(&pid, 10.0f, 1.0f, dt);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.8f, out2);
}

TEST(PID, DerivativeTermOnly)
{
    Control_PID_Init(&pid, 0.0f, 0.0f, 1.0f, &cfg);
    float dt = 0.1f;

    // Output:
    // error = 10.0f - 1.0f = 9.0f;
    // derivative = 9.0 / 0.1 = 90;
    // p_term = derivative * kd = 90 * 1.0 = 90;
    float out1 = Control_PID_Update(&pid, 10.0f, 1.0f, dt);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 90.0f, out1);

    // Output:
    // error = 10.0f - 1.0f = 9.0f;
    // derivative = (error - prev_error) / dt = (9 - 9) /dt = 0
    // p_term = derivative * kd = 0;
    float out2 = Control_PID_Update(&pid, 10.0f, 1.0f, dt);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, out2);

    // Output:
    // error = 5.0f - 0.0f = 5.0f;
    // derivative = (5.0f - 9.0f) / dt = -40.0f;
    // p_term = -40.0f * 1 = -40;
    float out3 = Control_PID_Update(&pid, 5.0f, 0.0f, dt);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -40.0f, out3);
}

TEST(PID, AntiWindupClampsToMax)
{
    float dt = 0.1f;

    cfg.enable_anti_windup = true;
    cfg.max_out            = 10.0f;
    cfg.min_out            = -INFINITY;
    Control_PID_Init(&pid, 1.0f, 0.0f, 0.0f, &cfg);

    // error = 20.0f - 0.0f = 20.0f
    // out = 20.0f * kp = max(10.0f, 20.0f) = 10.0f
    float out = Control_PID_Update(&pid, 20.0f, 0, dt);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 10.0f, out);
}

TEST(PID, AntiWindupClampsToMin)
{
    float dt = 0.1f;

    cfg.enable_anti_windup = true;
    cfg.max_out            = INFINITY;
    cfg.min_out            = 10.0f;
    Control_PID_Init(&pid, 1.0f, 0.0f, 0.0f, &cfg);

    // error = 1.0f - 0.0f = 1.0f
    // out = 1.0f * kp = 1.0f >>> 10.0f
    float out = Control_PID_Update(&pid, 1.0f, 0, dt);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 10.0f, out);
}

TEST_GROUP_RUNNER(PID)
{
    RUN_TEST_CASE(PID, ProportionalTermOnly);
    RUN_TEST_CASE(PID, IntegralTermOnly);
    RUN_TEST_CASE(PID, DerivativeTermOnly);
    RUN_TEST_CASE(PID, AntiWindupClampsToMax);
    RUN_TEST_CASE(PID, AntiWindupClampsToMin);
}
