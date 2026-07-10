#include "internal_common.h"
#include <ccontrol/arena.h>
#include <ccontrol/core.h>
#include <ccontrol/matrix.h>
#include <ccontrol/tf.h>
#include <stddef.h>

#ifndef CONTROL_PRIVATE_API
#define CONTROL_PRIVATE_API static
#endif

#ifndef CONTROL_INLINE_API
#define CONTROL_INLINE_API inline
#endif

#define REQUIRE_VALID_VEC(ctx, vec_ptr, msg)                                   \
    CCONTROL_REQUIRE(ctx, Vector_IsValid(vec_ptr),                             \
                     CCONTROL_ERROR_OUT_OF_MEMORY, msg, CCONTROL_EMPTY_VEC)

#define REQUIRE_VALID_TF(ctx, tf_ptr, code, msg)                               \
    CCONTROL_REQUIRE(ctx, TransferFunction_IsValid(tf_ptr), code, msg,         \
                     CCONTROL_EMPTY_TF)

void ControlSystem_InitHandle(ControlHandle *ctx, ControlArena *p,
                              ControlArena *s)
{
    ctx->persistent = p;
    ctx->scratch = s;
}

void ControlSystem_DeInitHandle(ControlHandle *ctx)
{
    ControlArena_Clear(ctx->scratch);
    ControlArena_Clear(ctx->persistent);
}

CONTROL_PRIVATE_API control_vector_t __CreateVectorInArena(ControlArena *a,
                                                           size_t capacity)
{
    control_vector_t v;
    v.capacity = 0;
    v.size = 0;
    v.coeffs = NULL;

    v.coeffs = (float *)ArenaAlloc(a, capacity * sizeof(float));
    if (!Vector_IsValid(&v))
    {
        return CCONTROL_EMPTY_VEC;
    }

    v.capacity = capacity;
    return v;
}

static control_vector_t
__PolyCoeffVector_InArena(ControlArena *a, const float *coeffs, size_t size)
{
    if (coeffs == NULL || size <= 0)
    {
        return CCONTROL_EMPTY_VEC;
    }

    control_vector_t v = __CreateVectorInArena(a, size);
    if (!Vector_IsValid(&v))
    {
        return CCONTROL_EMPTY_VEC;
    }

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
    control_vector_t v = __PolyCoeffVector_InArena(ctx->scratch, coeffs, size);
    REQUIRE_VALID_VEC(ctx, &v, "Could not allocate in scratch arena");

    return v;
}

control_vector_t PolyCoeffVector_Persistent(ControlHandle *ctx,
                                            const float *coeffs, size_t size)
{
    control_vector_t v =
        __PolyCoeffVector_InArena(ctx->persistent, coeffs, size);
    REQUIRE_VALID_VEC(ctx, &v, "Could not allocate in persistent arena");

    return v;
}

static control_vector_t __PolyCoeffAdd_InArena(ControlArena *arena,
                                               const control_vector_t *a,
                                               const control_vector_t *b)
{
    size_t max_size = a->size > b->size ? a->size : b->size;
    control_vector_t vec = __CreateVectorInArena(arena, max_size);
    if (!Vector_IsValid(&vec))
    {
        return vec;
    }

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
    control_vector_t v = __PolyCoeffAdd_InArena(ctx->scratch, a, b);
    REQUIRE_VALID_VEC(
        ctx, &v,
        "Could not create vector for polynomial addition in scratch arena");

    return v;
}

static control_vector_t __MultiplyPoly_InArena(ControlArena *arena,
                                               control_vector_t *a,
                                               control_vector_t *b)
{
    if (!Vector_IsValid(a) || !Vector_IsValid(b))
    {
        return CCONTROL_EMPTY_VEC;
    }
    size_t new_size = a->size + b->size - 1;

    control_vector_t result = __CreateVectorInArena(arena, new_size);
    if (!Vector_IsValid(&result))
    {
        return CCONTROL_EMPTY_VEC;
    }

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
    control_vector_t v = __MultiplyPoly_InArena(ctx->scratch, a, b);
    REQUIRE_VALID_VEC(ctx, &v, "Could not multiply polynomials");

    return v;
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
    if (!Vector_IsValid(&conv_num))
    {
        return CCONTROL_EMPTY_TF;
    }

    control_vector_t conv_dem = __MultiplyPoly_InArena(a, &G1->dem, &G2->dem);
    if (!Vector_IsValid(&conv_dem))
    {
        return CCONTROL_EMPTY_TF;
    }

    control_vector_t clean_num = PolyCoeffVector_Cannonicalize(&conv_num);
    control_vector_t clean_dem = PolyCoeffVector_Cannonicalize(&conv_dem);

    return TransferFunctionFromCoeffs(&clean_num, &clean_dem);
}

TransferFunction MultiplyTransferFunctions(ControlHandle *ctx,
                                           TransferFunction *G1,
                                           TransferFunction *G2)
{
    REQUIRE_VALID_TF(ctx, G1, CCONTROL_ERROR_INVALID_ARGUMENT,
                     "TransferFunction G1 is NULL or invalid");
    REQUIRE_VALID_TF(ctx, G2, CCONTROL_ERROR_INVALID_ARGUMENT,
                     "TransferFunction G2 is NULL or invalid");

    TransferFunction tf =
        __MultiplyTransferFunctions_InArena(ctx->scratch, G1, G2);
    REQUIRE_VALID_TF(ctx, &tf, CCONTROL_ERROR_OUT_OF_MEMORY,
                     "Could not multiply transfer function");
    return tf;
}

inline bool TransferFunction_IsValid(TransferFunction *tf)
{
    return tf != NULL && tf->num.coeffs != NULL && tf->dem.coeffs != NULL;
}

TransferFunction UnityClosedLoop(ControlHandle *ctx, TransferFunction *G,
                                 float gain, TransferFunctionUnity unity)
{
    REQUIRE_VALID_TF(ctx, G, CCONTROL_ERROR_DIVIDE_BY_ZERO,
                     "TransferFunction G is NULL or invalid");
    /*
     * G(s) = N(s)/D(s)
     *
     * H(s) = G(s)/(1 + k*G(s)) = (N(s)/D(s))/(1 + k*(N(s)/D(s))
     * H(s) = N(s)/(D(s) + k*N(s))
     */

    if (unity == TF_UNITY_NEGATIVE)
    {
        gain = -gain;
    }

    control_vector_t scaled_num = {0};
    if (gain != 1.0f)
    {
        scaled_num = __CreateVectorInArena(ctx->scratch, G->num.size);
        CCONTROL_REQUIRE(
            ctx, Vector_IsValid(&scaled_num), CCONTROL_ERROR_OUT_OF_MEMORY,
            "Failed to allocate vector in scratch arena", CCONTROL_EMPTY_TF);

        for (size_t i = 0; i < G->num.size; i++)
        {
            scaled_num.coeffs[i] = G->num.coeffs[i] * gain;
        }
    }
    else
    {
        scaled_num = G->num;
    }

    control_vector_t denom =
        __PolyCoeffAdd_InArena(ctx->scratch, &G->dem, &scaled_num);
    if (!Vector_IsValid(&denom))
    {
        CCONTROL_THROW(ctx, CCONTROL_ERROR_OUT_OF_MEMORY,
                       "Could not allocate vector for polynomial addition for "
                       "closed loop");
        return CCONTROL_EMPTY_TF;
    }

    return TransferFunctionFromCoeffs(&G->num, &denom);
}
