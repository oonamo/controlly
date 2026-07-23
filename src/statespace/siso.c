#include "../internal_common.h"
#include "ccontrol/matrix.h"
#include "ss_internal.h"
#include <ccontrol/core.h>
#include <ccontrol/statespace.h>

static ControlResult __gen_sys_matrix_InPersistent(ControlHandle *ctx,
                                                   ControlSystemMatrix *out,
                                                   const ControlTransferFunction *tf)
{
    size_t n = tf->den.size - 1;

    CCONTROL_TRY(Control_Matrix_AllocPersistent(ctx, out, n, n));

    for (size_t i = 0; i < n - 1; i++)
    {
        out->data[i * n + (i + 1)] = 1;
    }

    for (size_t i = 0; i < n; i++)
    {
        out->data[(n - 1) * n + i] = -tf->den.coeffs[n - i];
    }

    return CCONTROL_OK;
}

static ControlResult __gen_input_matrix_InPersistent(ControlHandle *ctx,
                                                     ControlInputMatrix *out,
                                                     const ControlTransferFunction *tf)
{
    size_t n = tf->den.size - 1;
    CCONTROL_TRY(Control_Matrix_AllocPersistent(ctx, out, n, 1));
    out->data[n - 1] = 1;
    return CCONTROL_OK;
}

static ControlResult __gen_output_matrix_InPersistent(ControlHandle *ctx,
                                                      ControlOutputMatrix *out,
                                                      const ControlTransferFunction *tf)
{
    size_t n = tf->den.size - 1;
    CCONTROL_TRY(Control_Matrix_AllocPersistent(ctx, out, 1, n));

    size_t m = tf->num.size - 1;

    float b0 = 0.0f;
    if (tf->num.size == tf->den.size)
    {
        b0 = tf->num.coeffs[0];
    }

    size_t offset = n - m;

    for (size_t i = 0; i < n; i++)
    {
        size_t k = n - i; // Polynomial order

        float ak = tf->den.coeffs[k];
        float bk = 0.0f;

        if (k >= offset)
        {
            bk = tf->num.coeffs[k - offset];
        }
        out->data[i] = bk - ak * b0;
    }
    return CCONTROL_OK;
}

static ControlResult __gen_feedthrough_matrix_InPersistent(ControlHandle *ctx,
                                                           ControlFeedbackMatrix *out,
                                                           const ControlTransferFunction *tf)
{
    CCONTROL_TRY(Control_Matrix_AllocPersistent(ctx, out, 1, 1));

    float b0 = 0.0f;
    if (tf->num.size == tf->den.size)
    {
        b0 = tf->num.coeffs[0];
    }

    out->data[0] = b0;
    return CCONTROL_OK;
}

ControlResult Control_StateSpace_FromTF(ControlHandle *ctx,
                                        ControlStateSpace *out,
                                        const ControlTransferFunction *tf)
{
    CHECK_CTX(ctx);
    CHECK_NOT_NULL(ctx, out && tf, "Passed null pointers");
    CCONTROL_TRY(__gen_sys_matrix_InPersistent(ctx, &out->A, tf));
    CCONTROL_TRY(__gen_input_matrix_InPersistent(ctx, &out->B, tf));
    CCONTROL_TRY(__gen_output_matrix_InPersistent(ctx, &out->C, tf));
    CCONTROL_TRY(__gen_feedthrough_matrix_InPersistent(ctx, &out->D, tf));

    return CCONTROL_OK;
}

#ifndef MAX_SYSTEM_ORDER
#    define MAX_SYSTEM_ORDER 10
#endif

void __Control_StateSpace_StepSISO(ControlStateSpace *ss, float dt)
{
    size_t n = ss->A.rows;

    float x_dot[MAX_SYSTEM_ORDER] = {0.0f};

    // Step 1:
    // y = Cx + Du

    ss->y.coeffs[0] = 0.0f;

    // Cx calculations
    for (size_t i = 0; i < n; i++)
    {
        ss->y.coeffs[0] += ss->C.data[i] * ss->x.coeffs[i];
    }

    // Du calculation
    ss->y.coeffs[0] += ss->D.data[0] * ss->u.coeffs[0];

    // Step 2:
    // x_dot = Ax + Bu
    for (size_t i = 0; i < n; i++)
    {
        x_dot[i] = 0.0f;

        for (size_t j = 0; j < n; j++)
        {
            x_dot[i] += ss->A.data[i * n + j] * ss->x.coeffs[j];
        }

        x_dot[i] += ss->B.data[i] * ss->u.coeffs[0];
    }

    // Euler Integration
    for (size_t i = 0; i < n; i++)
    {
        ss->x.coeffs[i] += x_dot[i] * dt;
    }
}
