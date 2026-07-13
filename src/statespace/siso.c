#include "../internal_common.h"
#include "ccontrol/matrix.h"
#include "ss_internal.h"
#include <ccontrol/core.h>
#include <ccontrol/statespace.h>

static ControlSystemMatrixj __generate_sys_matrix_InPersistent(ControlArena *persistent,
                                                               const ControlTransferFunction *tf)
{
    size_t n = tf->dem.size - 1;
    ControlMatrix A = Control_Matrix_Alloc(persistent, n, n);
    if (!Control_Matrix_IsValid(&A))
    {
        return CCONTROL_EMPTY_MATRIX;
    }

    for (size_t i = 0; i < n - 1; i++)
    {
        A.data[i * n + (i + 1)] = 1;
    }

    for (size_t i = 0; i < n; i++)
    {
        A.data[(n - 1) * n + i] = -tf->dem.coeffs[n - i];
    }

    return A;
}

static ControlInputMatrix __gen_input_matrix_InPersistent(ControlArena *persistent,
                                                          const ControlTransferFunction *tf)
{
    size_t n = tf->dem.size - 1;
    ControlMatrix B = Control_Matrix_Alloc(persistent, n, 1);
    if (!Control_Matrix_IsValid(&B))
    {
        return CCONTROL_EMPTY_MATRIX;
    }
    B.data[n - 1] = 1;
    return B;
}

static ControlOutputMatrix __gen_output_matrix_InPersistent(ControlArena *persistent,
                                                            const ControlTransferFunction *tf)
{
    size_t n = tf->dem.size - 1;
    ControlMatrix C = Control_Matrix_Alloc(persistent, 1, n);
    if (!Control_Matrix_IsValid(&C))
    {
        return CCONTROL_EMPTY_MATRIX;
    }

    size_t m = tf->num.size - 1;

    float b0 = 0.0f;
    if (tf->num.size == tf->dem.size)
    {
        b0 = tf->num.coeffs[0];
    }

    size_t offset = n - m;

    for (size_t i = 0; i < n; i++)
    {
        size_t k = n - i; // Polynomial order

        float ak = tf->dem.coeffs[k];
        float bk = 0.0f;

        if (k >= offset)
        {
            bk = tf->num.coeffs[k - offset];
        }
        C.data[i] = bk - ak * b0;
    }
    return C;
}

static ControlFeedbackMatrix
__gen_feedthrough_matrix_InPersistent(ControlArena *persistent, const ControlTransferFunction *tf)
{
    ControlFeedbackMatrix D = Control_Matrix_Alloc(persistent, 1, 1);
    if (!Control_Matrix_IsValid(&D))
    {
        return CCONTROL_EMPTY_MATRIX;
    }
    float b0 = 0.0f;
    if (tf->num.size == tf->dem.size)
    {
        b0 = tf->num.coeffs[0];
    }

    D.data[0] = b0;
    return D;
}

ControlStateSpace Control_StateSpace_FromTF(ControlHandle *ctx, ControlTransferFunction *tf)
{
    CCONTROL_REQUIRE(ctx,
                     tf != NULL,
                     CCONTROL_ERROR_INVALID_ARGUMENT,
                     "Passed NULL transfer function as parameter",
                     CCONTROL_EMPTY_STATESPACE);

    ControlSystemMatrixj A = __generate_sys_matrix_InPersistent(ctx->persistent, tf);
    CCONTROL_REQUIRE(ctx,
                     Control_Matrix_IsValid(&A),
                     CCONTROL_ERROR_OUT_OF_MEMORY,
                     "Could not generate A matrix",
                     CCONTROL_EMPTY_STATESPACE);

    ControlInputMatrix B = __gen_input_matrix_InPersistent(ctx->persistent, tf);
    CCONTROL_REQUIRE(ctx,
                     Control_Matrix_IsValid(&B),
                     CCONTROL_ERROR_OUT_OF_MEMORY,
                     "Could not generate B matrix",
                     CCONTROL_EMPTY_STATESPACE);

    ControlOutputMatrix C = __gen_output_matrix_InPersistent(ctx->persistent, tf);
    CCONTROL_REQUIRE(ctx,
                     Control_Matrix_IsValid(&C),
                     CCONTROL_ERROR_OUT_OF_MEMORY,
                     "Could not generate C matrix",
                     CCONTROL_EMPTY_STATESPACE);

    ControlFeedbackMatrix D = __gen_feedthrough_matrix_InPersistent(ctx->persistent, tf);
    CCONTROL_REQUIRE(ctx,
                     Control_Matrix_IsValid(&D),
                     CCONTROL_ERROR_OUT_OF_MEMORY,
                     "Could not generate D matrix",
                     CCONTROL_EMPTY_STATESPACE);

    ControlStateSpace s = {
        .A = A,
        .B = B,
        .C = C,
        .D = D,
    };
    return s;
}

#ifndef MAX_SYSTEM_ORDER
#define MAX_SYSTEM_ORDER 10
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
