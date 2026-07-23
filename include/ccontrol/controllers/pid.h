#ifndef _PID_H
#define _PID_H

#include <ccontrol/tf.h>
#include <stdbool.h>

typedef struct
{
    bool enable_anti_windup;
    float max_out;
    float min_out;
} ControlPIDConfig;

typedef struct
{
    float kp;
    float kd;
    float ki;

    float integral;
    float prev_error;

    ControlPIDConfig config;
} ControlPIDController;

void Control_PID_Init(
    ControlPIDController *pid, float kp, float ki, float kd, ControlPIDConfig *config);

void Control_PID_Reset(ControlPIDController *pid);

float Control_PID_Update(ControlPIDController *pid, float target, float measurment, float dt);

ControlResult Control_PID_ToTF(ControlHandle *ctx, ControlTransferFunction* out, ControlPIDController *pid);
#endif
