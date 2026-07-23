/**
 * @file pid.h
 * @brief Proportional-Integral-Derivative (PID) controller module.
 *
 * This module provides a standard PID controller implementation with
 * optional anti-windup and output clamping features.
 */
#ifndef _CCONTROL_PID_H
#define _CCONTROL_PID_H

#include <ccontrol/tf.h>
#include <stdbool.h>

/**
 * @brief Configuration structure for PID controller
 */
typedef struct
{
    bool  enable_anti_windup; /*!< If enabled, clamps integral accumulator to prevent windup */
    float max_out;            /*!< Maximum allowable output (if `enable_anti_windup` is enabled) */
    float min_out;            /*!< Minimum allowable output (if `enable_anti_windup` is enabled) */
} ControlPIDConfig;

/**
 * @brief PID Controller state and configuration structure
 */
typedef struct
{
    float kp; /*!< Proportional gain */
    float kd; /*!< Derivative gain */
    float ki; /*!< Integral gain */

    float integral;   /*!< Internal integral accumulator for the integral term */
    float prev_error; /*!< Internal memory of previous error for derivative gain */

    ControlPIDConfig config; /*!< PID configuration structure */
} ControlPIDController;

/**
 * @brief Initializes a PID controller with the specified gains and configuration.
 *
 * @param[out] pid    Pointer to the PID controller to initialize.
 * @param[in]  kp     Proportional gain.
 * @param[in]  ki     Integral gain.
 * @param[in]  kd     Derivative gain.
 * @param[in]  config Pointer to the configuration struct. If NULL, anti-windup and limits are
 * disabled.
 */
void Control_PID_Init(
    ControlPIDController *pid, float kp, float ki, float kd, const ControlPIDConfig *config);

/**
 * @brief Resets the internal state of the PID controller.
 *
 * Clears the accumulated integral and previous error. This should be called
 * whenever the controller is re-enabled or the setpoint changes drastically.
 *
 * @param[in,out] pid Pointer to the initialized PID controller.
 */
void Control_PID_Reset(ControlPIDController *pid);

/**
 * @brief Computes the next control effort for the PID controller.
 *
 * Calculates the error (`target - measurement`) and updates the internal
 * integral and derivative states using Euler integration and differentiation.
 *
 * @param[in,out] pid         Pointer to the initialized PID controller.
 * @param[in]     target      The desired setpoint.
 * @param[in]     measurement The current measured state of the system.
 * @param[in]     dt          Continuous time step (delta time) since the last update.
 *
 * @return The computed control effort, clamped to the configured limits (if applicable).
 */
float Control_PID_Update(ControlPIDController *pid, float target, float measurement, float dt);

/**
 * @brief Converts a PID controller's gains into a continuous-time Transfer Function.
 *
 * Maps the PID gains to standard polynomial form: $C(s) = \frac{K_d s^2 + K_p s + K_i}{s}$.
 *
 * @note Allocates from the \p ctx->persistent arena.
 *       **Zero-Allocation Path:** If the `out` transfer function already possesses
 *       sufficient capacity in both its numerator and denominator vectors, no memory is allocated.
 *
 * @param[in,out] ctx Pointer to the initialized control context.
 * @param[out]    out Pointer to the destination transfer function.
 * @param[in]     pid Pointer to the source PID controller.
 *
 * @return CCONTROL_OK on success.
 * @return CCONTROL_ERROR_OUT_OF_MEMORY if the persistent arena is exhausted.
 * @return CCONTROL_ERROR_NULL_PTR if any pointer is NULL.
 */
ControlResult
Control_PID_ToTF(ControlHandle *ctx, ControlTransferFunction *out, const ControlPIDController *pid);

#endif // _CCONTROL_PID_H
