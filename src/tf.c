#include <ccontrol/arena.h>
#include <ccontrol/matrix.h>
#include <ccontrol/tf.h>
#include <stddef.h>

#ifndef CONTROL_PRIVATE_API
#define CONTROL_PRIVATE_API static
#endif

#ifndef CONTROL_INLINE_API
#define CONTROL_INLINE_API inline
#endif

void ControlSystem_InitHandle(ControlHandle *ctx, ControlArena *p,
                              ControlArena *s)
{
    ctx->persistent = p;
    ctx->scratch = s;
}

void ControlSystemInit(void *persistentData, void *temporaryData) {}

void ControlSystem_DeInitHandle(ControlHandle *ctx){}

#define NULL_VECTOR                                                            \
    (control_vector_t) { .capacity = 0, .size = 0, .coeffs = NULL }

CONTROL_PRIVATE_API control_vector_t
    __CreateVectorInArena(ControlArena *a, size_t capacity)
{
    control_vector_t v;
    v.capacity = 0;
    v.size = 0;
    v.coeffs = NULL;

    void *raw_mem = (float *)ArenaAlloc(a, capacity * sizeof(float));
    if (raw_mem == NULL)
    {
        // TODO: Call handle and error
        return v;
    }

    v.capacity = capacity;
    v.coeffs = (float *)raw_mem;
    return v;
}

static control_vector_t
__PolyCoeffVector_InArena(ControlArena *a, const float *coeffs, size_t size)
{
    if (coeffs == NULL || size <= 0)
    {
        return NULL_VECTOR;
    }

    control_vector_t v = __CreateVectorInArena(a, size);
    v.size = size;

    for (size_t i = 0; i < size; i++)
    {
        v.coeffs[i] = coeffs[i];
    }

    return v;
}

control_vector_t PolyCoeffVector_Cannonicalize(const control_vector_t *v)
{
    control_vector_t canon = *v;

    size_t i = 0;
    while (i < v->size - 1 && v->coeffs[i] == 0.0f)
    {
        i++;
    }

    canon.coeffs = &v->coeffs[i];
    canon.size = v->size - i;

    return canon;
}

/**
 * @brief Creates a vector of polynomial coefficients
 *
 * @param coeffs Polynomial coefficients starting from n -> 0
 * @param size size of coefficients
 * @return Vector representation
 */
control_vector_t PolyCoeffVector_Scratch(ControlHandle *ctx,
                                         const float *coeffs, size_t size)
{
    return __PolyCoeffVector_InArena(ctx->scratch, coeffs, size);
}

control_vector_t PolyCoeffVector_Persistent(ControlHandle *ctx,
                                            const float *coeffs, size_t size)
{
    return __PolyCoeffVector_InArena(ctx->persistent, coeffs, size);
}

CONTROL_PRIVATE_API char __ControlsUnsafeIntToChar(int i)
{
    return (char)(i + '0');
}

CONTROL_INLINE_API CONTROL_PRIVATE_API int
__ControlsSetChar(char c, char *buffer, size_t buffer_size)
{
    if ((buffer + 1) > (buffer + buffer_size))
    {
        return 0;
    }

    *buffer = c;
    return 1;
}

CONTROL_PRIVATE_API int __ControlsUnsafeAppendStr(char *buffer, size_t buflen,
                                                  char *src)
{
    int last_result = 1;
    int i = 0;
    while (last_result && src[i] != '\0')
    {
        last_result = __ControlsSetChar(src[i], buffer + i, buflen);
        i++;
    }

    return i;
}

CONTROL_INLINE_API CONTROL_PRIVATE_API int
__ControlsAppendStr(char *buffer, size_t buflen, char *src, size_t srclen)
{
    int last_result = 1;
    int i = 0;
    while (last_result && i < srclen)
    {
        last_result = __ControlsSetChar(src[i++], buffer, buflen);
    }
    return i;
}

CONTROL_PRIVATE_API void __ControlReverseStr(char *buf, size_t len)
{
    int i = 0, j = len - 1, tmp;
    while (i < j)
    {
        tmp = buf[i];
        buf[i] = buf[j];
        buf[j] = tmp;
        i++;
        j--;
    }
}

CONTROL_PRIVATE_API int __ControlsIntToStr(int x, char *buffer, size_t ndigits)
{
    size_t i = 0;
    if (x == 0)
    {
        buffer[i++] = '0';
    }
    while (x)
    {
        buffer[i++] = (x % 10) + '0';
        x = x / 10;
    }
    while (i < ndigits)
    {
        buffer[i++] = '0';
    }

    __ControlReverseStr(buffer, i);
    buffer[i] = '\0';
    return i;
}

CONTROL_INLINE_API CONTROL_PRIVATE_API int __ControlsFTOA(float n, char *res,
                                                          size_t precision)
{
    int isneg = 0;
    int i = 0;

    if (n < 0.0f)
    {
        n = -n;
        isneg = 1;
    }

    int integral_part = (int)n;
    float float_part = n - (float)integral_part;
    int i_len = __ControlsIntToStr(integral_part, res + i, 0);
    i += i_len;

    if (precision != 0)
    {
        res[i++] = '.';

        float multiplier = 1.0f;
        for (size_t p = 0; p < precision; p++)
        {
            multiplier *= 10.0f;
        }

        int fractional_int = (int)(float_part * multiplier + 0.5f);

        __ControlsIntToStr(fractional_int, res + i, precision);
    }

    return isneg;
}

int PolyCoeffVectorToStr(const control_vector_t *coeffs, char var, char *buffer,
                         size_t buffer_size)
{
    size_t buffer_ptr = 0;
    size_t coeff_size = coeffs->size;
    int order = (int)(coeff_size)-1;

    char float_buf[16];
    char order_buf[16];

    for (int i = 0; i < coeff_size; i++)
    {
        float coeff = coeffs->coeffs[i];

        if (coeff == 0.0f && coeff_size != 1)
        {
            continue;
        }

        if (i != 0 && coeff < 0)
        {
            buffer_ptr += __ControlsUnsafeAppendStr(buffer + buffer_ptr,
                                                    buffer_size, " - ");
        }
        else if (i != 0)
        {
            buffer_ptr += __ControlsUnsafeAppendStr(buffer + buffer_ptr,
                                                    buffer_size, " + ");
        }

        int isneg = __ControlsFTOA(coeff, float_buf, 2);
        if (isneg && i == 0)
        {
            buffer_ptr += __ControlsUnsafeAppendStr(buffer + buffer_ptr,
                                                    buffer_size, "-");
        }

        buffer_ptr += __ControlsUnsafeAppendStr(buffer + buffer_ptr,
                                                buffer_size, float_buf);

        if (order > 1)
        {
            buffer_ptr += __ControlsUnsafeAppendStr(buffer + buffer_ptr,
                                                    buffer_size, "s^");
            __ControlsIntToStr(order, order_buf, 0);
            buffer_ptr += __ControlsUnsafeAppendStr(buffer + buffer_ptr,
                                                    buffer_size, order_buf);
        }
        else if (order == 1)
        {
            buffer_ptr += __ControlsUnsafeAppendStr(buffer + buffer_ptr,
                                                    buffer_size, "s");
        }

        order--;
    }

    buffer[buffer_ptr] = '\0';

    return order != 0;
}

int TransferFunctionToStr(TransferFunction *tf, char var, char *buffer,
                          size_t buffer_size)
{
    size_t i = 0;
    __ControlsSetChar('(', buffer + i++, buffer_size);
    PolyCoeffVectorToStr(&tf->num, var, buffer + i, buffer_size);
    while (buffer[i] != '\0')
    {
        i++;
    }
    i += __ControlsUnsafeAppendStr(buffer + i, buffer_size, ")/(");
    PolyCoeffVectorToStr(&tf->dem, var, buffer + i, buffer_size);
    while (buffer[i] != '\0')
    {
        i++;
    }
    __ControlsSetChar(')', buffer + i++, buffer_size);

    buffer[i] = '\0';

    return 0;
}

static control_vector_t __PolyCoeffAdd_InArena(ControlArena *arena,
                                               const control_vector_t *a,
                                               const control_vector_t *b)
{
    size_t max_size = a->size > b->size ? a->size : b->size;
    control_vector_t vec = __CreateVectorInArena(arena, max_size);

    size_t a_offset = max_size - a->size;
    size_t b_offset = max_size - b->size;

    for (size_t i = 0; i < max_size; i++)
    {
        float sum = 0.0f;
        if (i >= a_offset)
        {
            sum += a->coeffs[i - a_offset];
        }
        if (i >= b_offset)
        {
            sum += b->coeffs[i - b_offset];
        }

        vec.coeffs[i] = sum;
    }

    vec.size = max_size;
    return vec;
}

control_vector_t AddCoeffVector(ControlHandle *ctx, const control_vector_t *a,
                                const control_vector_t *b)
{
    return __PolyCoeffAdd_InArena(ctx->scratch, a, b);
}

static control_vector_t __MultiplyPoly_InArena(ControlArena *arena,
                                               control_vector_t *a,
                                               control_vector_t *b)
{
    size_t new_size = a->size + b->size - 1;

    control_vector_t result = __CreateVectorInArena(arena, new_size);
    result.size = new_size;

    for (size_t i = 0; i < new_size; i++)
    {
        result.coeffs[i] = 0.0f;
    }

    for (size_t i = 0; i < a->size; i++)
    {
        for (size_t j = 0; j < b->size; j++)
        {
            result.coeffs[i + j] += a->coeffs[i] * b->coeffs[j];
        }
    }

    return result;
}

control_vector_t MultiplyPoly(ControlHandle *ctx, control_vector_t *a,
                              control_vector_t *b)
{
    return __MultiplyPoly_InArena(ctx->scratch, a, b);
}

TransferFunction TransferFunctionFromCoeffs(const control_vector_t *num,
                                            const control_vector_t *dem)
{
    TransferFunction G = {
        .num = *num,
        .dem = *dem,
    };
    return G;
}

static TransferFunction
__MultiplyTransferFunctions_InArena(ControlArena *a, TransferFunction *G1,
                                    TransferFunction *G2)
{
    control_vector_t conv_num = __MultiplyPoly_InArena(a, &G1->num, &G2->num);
    control_vector_t conv_dem = __MultiplyPoly_InArena(a, &G1->dem, &G2->dem);

    control_vector_t clean_num = PolyCoeffVector_Cannonicalize(&conv_num);
    control_vector_t clean_dem = PolyCoeffVector_Cannonicalize(&conv_dem);

    return TransferFunctionFromCoeffs(&clean_num, &clean_dem);
}

TransferFunction MultiplyTransferFunctions(ControlHandle *ctx,
                                           TransferFunction *G1,
                                           TransferFunction *G2)
{
    return __MultiplyTransferFunctions_InArena(ctx->scratch, G1, G2);
}

TransferFunction UnityClosedLoop(ControlHandle *ctx, TransferFunction *G,
                                 float gain, TransferFunctionUnity unity)
{
    /*
     * G(s) = N(s)/D(s)
     *
     * H(s) = G(s)/(1 + G(s)) = (N(s)/D(s))/(1 + (N(s)/D(s))
     * H(s) = N(s)/(D(s) + N(s))
     */

    control_vector_t denom = AddCoeffVector(ctx, &G->dem, &G->num);

    return TransferFunctionFromCoeffs(&G->num, &denom);
}

// Assume matrix is strictly proper
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
    vector_t B = ArenaAllocVec(persistent, n);
    B.coeffs[n - 1] = 1;
    return B;
}

static output_matrix_t
__gen_output_matrix_InPersistent(ControlArena *persistent,
                                 const TransferFunction *tf)
{
    size_t n = tf->dem.size - 1;
    vector_t C = ArenaAllocVec(persistent, n);

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
        C.coeffs[i] = bk - ak * b0;
    }
    return C;
}

static feedback_matrix_t
__gen_feedthrough_matrix_InPersistent(ControlArena *persistent,
                                      const TransferFunction *tf)
{
    vector_t D = ArenaAllocVec(persistent, 1);
    float b0 = 0.0f;
    if (tf->num.size == tf->dem.size)
    {
        b0 = tf->num.coeffs[0];
    }

    D.coeffs[0] = b0;
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

void StateSpace_StepContinous(ControlHandle *ctx, StateSpace *ss, float dt)
{
    size_t n = ss->A.rows;
    size_t m = ss->B.size;
    size_t p = ss->C.size;

    float x_dot[MAX_SYSTEM_ORDER] = {0.0f};
    ss->y.coeffs[0] = 0.0f;

    for (size_t i = 0; i < n; i++)
    {
        ss->y.coeffs[0] += ss->C.coeffs[i] * ss->x.coeffs[i];
    }

    ss->y.coeffs[0] += ss->D.coeffs[0] * ss->u.coeffs[0];

    for (size_t i = 0; i < n; i++)
    {
        x_dot[i] = 0.0f;

        // Matrix dot product: A row i * x vector
        for (size_t j = 0; j < n; j++)
        {
            x_dot[i] += ss->A.data[i * n + j] * ss->x.coeffs[j];
        }

        // Vector scaling: B[i] * u[0]
        x_dot[i] += ss->B.coeffs[i] * ss->u.coeffs[0];
    }

    for (size_t i = 0; i < n; i++)
    {
        ss->x.coeffs[i] = ss->x.coeffs[i] + (x_dot[i] * dt);
    }
}
