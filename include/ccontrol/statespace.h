/**
 * @file statespace.h
 * @brief State space module of CControl
 *
 * This module contains support for modeling of state space systems
 */
#ifndef _CCONTROL_STATESPACE_H
#define _CCONTROL_STATESPACE_H

#include <ccontrol/core.h>
#include <ccontrol/matrix.h>
#include <ccontrol/tf.h>

typedef ControlMatrix ControlSystemMatrix;
typedef ControlMatrix ControlInputMatrix;
typedef ControlMatrix ControlOutputMatrix;
typedef ControlMatrix ControlFeedbackMatrix;

/**
 * @brief A structure holding information on the State Space systems equation.
 */
typedef struct
{
    ControlSystemMatrix   A; /*!< The system matrix */
    ControlInputMatrix    B; /*!< The input matrix */
    ControlOutputMatrix   C; /*!< The output matrix */
    ControlFeedbackMatrix D; /*!< The feedback matrix */

    ControlVec y; /*!< The output vector */
    ControlVec u; /*!< The input vector  */
    ControlVec x; /*!< The state vector */
} ControlStateSpace;

/**
 * @brief Helper function for initializing an empty State Space structure.
 */
#define CCONTROL_EMPTY_STATESPACE (ControlStateSpace){0}

/**
 * @brief Mathematically steps into the next iteration of the state space equation.
 *
 * @note Both SISO and MIMO systems are supported.
 *
 * @note Uses Euler integration.
 *
 * @warning Large `dt` will result in non-continuous like behavior. It is recommended to clamp `dt`
 * to an appropriate value.
 *
 * @param[in,out] ctx Pointer to the initialized control context.
 * @param[in,out] ss  Pointer to the ControlStateSpace model.
 * @param[in]     dt  Continuous time step to use.
 */
void Control_StateSpace_StepContinuous(ControlHandle *ctx, ControlStateSpace *ss, float dt);

/**
 * @brief Transforms a ControlTransferFunction into a ControlStateSpace
 *
 * This function uses the **Controllable Canonical Form** model to convert the transfer function
 * into a state space system.
 *
 * @note Performs allocation in the `ctx->persistent` arena.
 *
 * @param[in,out] ctx Pointer to the initialized control context.
 * @param[out]    out Pointer to the destination ControlStateSpace.
 * @param[in]     tf  Pointer to the transform function to convert.
 *
 * @return CCONTROL_OK on success.
 * @return CCONTROL_ERROR_OUT_OF_MEMORY if the persistent arena is exhausted.
 * @return CCONTROL_ERROR_INVALID_ARGUMENT if the transfer function is invalid or improper.
 * @return CCONTROL_ERROR_NULL_PTR if any pointer is NULL.
 */
ControlResult Control_StateSpace_FromTF(ControlHandle                 *ctx,
                                        ControlStateSpace             *out,
                                        const ControlTransferFunction *tf);

#endif // _CCONTROL_STATESPACE_H
