#include "../internal_common.h"
#include <ccontrol/controllers/pid.h>
#include <ccontrol/matrix.h>
#include <ccontrol/tf.h>
#include <stddef.h>

void Control_PID_Init(
    ControlPIDController *pid, float kp, float ki, float kd, const ControlPIDConfig *config)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->integral   = 0.0f;
    pid->prev_error = 0.0f;

    if (config != NULL)
    {
        pid->config = *config;
    }
    else
    {
        pid->config.enable_anti_windup = false;
        pid->config.max_out            = 0.0f;
        pid->config.min_out            = 0.0f;
    }
}

void Control_PID_Reset(ControlPIDController *pid)
{
    pid->integral   = 0.0f;
    pid->prev_error = 0.0f;
}

float Control_PID_Update(ControlPIDController *pid, float setpoint, float measurement, float dt)
{
    if (dt <= 0.0f)
    {
        return 0.0f;
    }

    float error = setpoint - measurement;

    float p_term = pid->kp * error;

    pid->integral += error * dt;

    float i_term = pid->ki * pid->integral;

    float derivative = (error - pid->prev_error) / dt;
    float d_term     = pid->kd * derivative;

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

ControlResult
Control_PID_ToTF(ControlHandle *ctx, ControlTransferFunction *out, const ControlPIDController *pid)
{
    float num[3] = {pid->kd, pid->kp, pid->ki};
    float den[3] = {1.0f, 0.0f};

    ControlVec num_v = {0};
    ControlVec den_v = {0};

    CCONTROL_TRY(Control_Poly_AllocPersistent(ctx, &num_v, num, 3));
    CCONTROL_TRY(Control_Poly_AllocPersistent(ctx, &den_v, den, 3));

    CCONTROL_TRY(Control_TF_FromPoly(ctx, out, &num_v, &den_v));

    return CCONTROL_OK;
}
