#include "../internal_common.h"
#include <ccontrol/controllers/pid.h>
#include <ccontrol/matrix.h>
#include <ccontrol/tf.h>
#include <stddef.h>

void Control_PID_Init(
    ControlPIDController *pid, float kp, float ki, float kd, ControlPIDConfig *config)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->integral = 0.0f;
    pid->prev_error = 0.0f;

    if (config != NULL)
    {
        pid->config = *config;
    }
    else
    {
        pid->config.enable_anti_windup = false;
        pid->config.max_out = 0.0f;
        pid->config.min_out = 0.0f;
    }
}

void Control_PID_Reset(ControlPIDController *pid)
{
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
}

float Control_PID_Update(ControlPIDController *pid, float setpoint, float measurment, float dt)
{
    if (dt <= 0.0f)
    {
        return 0.0f;
    }

    float error = setpoint - measurment;

    float p_term = pid->kp * error;

    pid->integral += error * dt;

    float i_term = pid->ki * pid->integral;

    float derivative = (error - pid->prev_error) / dt;
    float d_term = pid->kd * derivative;

    float output = p_term + i_term + d_term;

    if (pid->config.enable_anti_windup)
    {
        if (output > pid->config.max_out)
        {
            output = pid->config.max_out;
            // Undo integral operation to reduce windup
            pid->integral -= error * dt;
        }
        else if (output < pid->config.min_out)
        {
            output = pid->config.min_out;
            pid->integral -= error * dt;
        }
    }

    pid->prev_error = error;

    return output;
}

ControlTransferFunction Control_PID_ToTF(ControlHandle *ctx, ControlPIDController *pid)
{
    float num[3] = {pid->kd, pid->kp, pid->ki};
    float den[3] = {1.0f, 0.0f};

    ControlVec num_v = Control_Poly_AllocPersistent(ctx, num, 3);
    CCONTROL_REQUIRE(ctx,
                     Control_Vec_IsValid(&num_v),
                     CCONTROL_ERROR_OUT_OF_MEMORY,
                     "Could not allocate memory for pid conversion",
                     CCONTROL_EMPTY_TF);

    ControlVec den_v = Control_Poly_AllocPersistent(ctx, den, 2);
    CCONTROL_REQUIRE(ctx,
                     Control_Vec_IsValid(&den_v),
                     CCONTROL_ERROR_OUT_OF_MEMORY,
                     "Could not allocate memory for pid conversion",
                     CCONTROL_EMPTY_TF);

    ControlTransferFunction tf = Control_TF_FromPoly(&num_v, &den_v);

    return tf;
}
