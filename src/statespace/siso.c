#include <ccontrol/statespace.h>
#include "ss_internal.h"

static system_matrix_t
__generate_sys_matrix_InPersistent(ControlArena *persistent,
                                   const TransferFunction *tf)
{
    size_t n = tf->dem.size - 1;
    matrix_t A = ArenaAllocMatrix(persistent, n, n);

    for (size_t i = 0; i < n - 1; i++)
    {
        A.data[i * n + (i + 1)] = 1;
    }

    size_t last_row_offset = (n - 1) * n;
    for (size_t i = 0; i < n; i++)
    {
        A.data[(n - 1) * n + i] = -tf->dem.coeffs[n - i];
    }

    return A;
}

static input_matrix_t
__gen_input_matrix_InPersistent(ControlArena *persistent,
                                const TransferFunction *tf)
{
    size_t n = tf->dem.size - 1;
    matrix_t B = ArenaAllocMatrix(persistent, n, 1);
    B.data[n - 1] = 1;
    return B;
}

static output_matrix_t
__gen_output_matrix_InPersistent(ControlArena *persistent,
                                 const TransferFunction *tf)
{
    size_t n = tf->dem.size - 1;
    matrix_t C = ArenaAllocMatrix(persistent, 1, n);

    size_t m = tf->num.size - 1;

    float b0 = 0.0f;
    if (tf->num.size == tf->dem.size)
    {
        b0 = tf->num.coeffs[0];
    }

    size_t offset = n - m;

    for (int i = 0; i < n; i++)
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

static feedback_matrix_t
__gen_feedthrough_matrix_InPersistent(ControlArena *persistent,
                                      const TransferFunction *tf)
{
    feedback_matrix_t D = ArenaAllocMatrix(persistent, 1, 1);
    float b0 = 0.0f;
    if (tf->num.size == tf->dem.size)
    {
        b0 = tf->num.coeffs[0];
    }

    D.data[0] = b0;
    return D;
}

StateSpace TransferFunctionToStateSpace(ControlHandle *ctx,
                                        TransferFunction *tf)
{
    system_matrix_t A = __generate_sys_matrix_InPersistent(ctx->persistent, tf);
    input_matrix_t B = __gen_input_matrix_InPersistent(ctx->persistent, tf);
    output_matrix_t C = __gen_output_matrix_InPersistent(ctx->persistent, tf);
    feedback_matrix_t D =
        __gen_feedthrough_matrix_InPersistent(ctx->persistent, tf);

    StateSpace s = {
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

void __StateSpace_StepSISO(ControlHandle *ctx, StateSpace *ss, float dt)
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

// void StateSpace_StepContinous(ControlHandle *ctx, StateSpace *ss, float dt)
// {
//     size_t n = ss->A.rows;
//     size_t m = ss->B.size;
//     size_t p = ss->C.size;
//
//     float x_dot[MAX_SYSTEM_ORDER] = {0.0f};
//     ss->y.coeffs[0] = 0.0f;
//
//     for (size_t i = 0; i < n; i++)
//     {
//         ss->y.coeffs[0] += ss->C.coeffs[i] * ss->x.coeffs[i];
//     }
//
//     ss->y.coeffs[0] += ss->D.coeffs[0] * ss->u.coeffs[0];
//
//     for (size_t i = 0; i < n; i++)
//     {
//         x_dot[i] = 0.0f;
//
//         // Matrix dot product: A row i * x vector
//         for (size_t j = 0; j < n; j++)
//         {
//             x_dot[i] += ss->A.data[i * n + j] * ss->x.coeffs[j];
//         }
//
//         // Vector scaling: B[i] * u[0]
//         x_dot[i] += ss->B.coeffs[i] * ss->u.coeffs[0];
//     }
//
//     for (size_t i = 0; i < n; i++)
//     {
//         ss->x.coeffs[i] = ss->x.coeffs[i] + (x_dot[i] * dt);
//     }
// }
