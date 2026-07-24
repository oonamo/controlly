#include "ss_internal.h"
#include <controlly/statespace.h>

#ifndef MAX_SYSTEM_ORDER
    #define MAX_SYSTEM_ORDER 10
#endif

void __Control_StateSpace_StepMIMO(ControlStateSpace *ss, float dt)
{
    size_t n = ss->A.rows; // num states  (n x n)
    size_t m = ss->B.cols; // num inputs  (n x m)
    size_t p = ss->C.rows; // num outputs (p x n)

    float x_dot[MAX_SYSTEM_ORDER] = {0.0f};

    // Output
    // y = Cx + Du
    for (size_t i = 0; i < p; i++)
    {
        ss->y.coeffs[i] = 0.0f;

        // C(p x n) * x (n x 1)
        for (size_t j = 0; j < n; j++)
        {
            ss->y.coeffs[i] += ss->C.data[i * n + j] * ss->x.coeffs[j];
        }

        // D(p x m) * u (m x 1)
        for (size_t j = 0; j < m; j++)
        {
            ss->y.coeffs[i] += ss->D.data[i * m + j] * ss->u.coeffs[j];
        }
    }

    // State
    for (size_t i = 0; i < n; i++)
    {
        x_dot[i] = 0.0f;

        // A(n x n) * x(n x 1)
        for (size_t j = 0; j < n; j++)
        {
            x_dot[i] += ss->A.data[i * n + j] * ss->x.coeffs[j];
        }

        // B(n x m) * u (m x 1)
        for (size_t j = 0; j < m; j++)
        {
            x_dot[i] += ss->B.data[i * n + j] * ss->u.coeffs[j];
        }
    }

    // Euler Integration
    for (size_t i = 0; i < n; i++)
    {
        ss->x.coeffs[i] += x_dot[i] * dt;
    }
}
