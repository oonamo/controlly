#include "tf.h"
#include "arena.h"

#ifndef CONTROL_PRIVATE_API
#define CONTROL_PRIVATE_API static
#endif

#ifndef CONTROL_INLINE_API
#define CONTROL_INLINE_API inline
#endif

#ifndef CONTROL_PERSISTENT_MEMORY_SIZE
#define CONTROL_PERSISTENT_MEMORY_SIZE 1024
#endif

#ifndef CONTROL_SCRATCH_MEMORY_SIZE
#define CONTROL_SCRATCH_MEMORY_SIZE 1024
#endif

/*
 * TODO: 2 different api's
 * 1. Embedded targets
 *  a. Allows for specifying allocation technique
 *    i. If it has dynamic memory allocation, let user use it
 *    ii. Let user provide static allocation
 * 2. General Targets
 *  a. Allow user to specify allocation function
 *    i. Allows user to specify that they wish for dynamic alloctation
 */

/*
 * Used for persisten memory
 * Guaranteed to be safe up to CONTROL_PERSISTENT_MEMORY_SIZE size
 */
Arena persistent_arena;
uint8_t persistent_buffer[CONTROL_PERSISTENT_MEMORY_SIZE];

/*
 * **VOLATAILE**
 * Used for intermediate results
 * To be reset after calculations
 */
Arena scratch_arena;
uint8_t scratch_buffer[CONTROL_SCRATCH_MEMORY_SIZE];

void ControlSystemInit()
{
    ArenaInit(&scratch_arena, scratch_buffer, CONTROL_SCRATCH_MEMORY_SIZE);
    ArenaInit(&persistent_arena, persistent_buffer,
              CONTROL_PERSISTENT_MEMORY_SIZE);
}

#define NULL_VECTOR                                                            \
    (control_vector_t) { .capacity = 0, .size = 0, .coeffs = NULL }

CONTROL_PRIVATE_API control_vector_t __CreateVectorInArena(Arena *a,
                                                           size_t capacity)
{
    control_vector_t v;
    v.capacity = capacity;
    v.size = 0;

    v.coeffs = (float *)ArenaAlloc(a, capacity * sizeof(float));
    return v;
}

/**
 * @brief Creates a vector of polynomial coefficients
 *
 * @param coeffs Polynomial coefficients starting from n -> 0
 * @param size size of coefficients
 * @return Vector representation
 */
control_vector_t PolyCoeffVector(float *coeffs, size_t size)
{
    if (coeffs == NULL || size <= 0)
    {
        return NULL_VECTOR;
    }

    control_vector_t v = __CreateVectorInArena(&persistent_arena, size);
    v.size = size;

    for (size_t i = 0; i < size; i++)
    {
        v.coeffs[i] = coeffs[i];
    }

    return v;
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

int PolyCoeffVectorToStr(control_vector_t *coeffs, char var, char *buffer,
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

control_vector_t AddCoeffVector(Arena *arena, control_vector_t *a,
                                control_vector_t *b)
{
    size_t max_size = a->size > b->size ? a->size : b->size;
    control_vector_t vec = __CreateVectorInArena(arena, max_size);

    for (size_t i = 0; i < max_size; i++)
    {
        float sum = 0.0f;
        if (i < a->size)
        {
            sum += a->coeffs[i];
        }
        if (i < b->size)
        {
            sum += a->coeffs[i];
        }

        vec.coeffs[i] = sum;
    }

    return vec;
}

control_vector_t ConvolveCoeffVector(Arena *arena, control_vector_t *a,
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

TransferFunction TransferFunctionFromCoeffs(control_vector_t num,
                                            control_vector_t dem)
{
    TransferFunction G = {
        .num = num,
        .dem = dem,
    };
    return G;
}

TransferFunction MultiplyTransferFunctrions(TransferFunction *G1,
                                            TransferFunction *G2)
{
    control_vector_t conv_num =
        ConvolveCoeffVector(&persistent_arena, &G1->num, &G2->num);
    control_vector_t conv_dem =
        ConvolveCoeffVector(&persistent_arena, &G1->dem, &G2->dem);

    return TransferFunctionFromCoeffs(conv_num, conv_dem);
}

TransferFunction ClosedLoopTransferFunction(TransferFunction *G, float gain,
                                            TransferFunctionUnity unity)
{
    /*
     * G(s) = N(s)/D(s)
     *
     * H(s) = G(s)/(1 + G(s)) = (N(s)/D(s))/(1 + (N(s)/D(s))
     * H(s) = N(s)/(D(s) + N(s))
     */

    control_vector_t denom =
        AddCoeffVector(&persistent_arena, &G->dem, &G->num);

    return TransferFunctionFromCoeffs(G->num, denom);
}
