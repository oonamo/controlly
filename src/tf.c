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

#define REQUIRE_VALID_VEC(ctx, vec_ptr, msg)                                                       \
    CCONTROL_REQUIRE(                                                                              \
        ctx, Control_Vec_IsValid(vec_ptr), CCONTROL_ERROR_OUT_OF_MEMORY, msg, CCONTROL_EMPTY_VEC)

#define REQUIRE_VALID_TF(ctx, tf_ptr, code, msg)                                                   \
    CCONTROL_REQUIRE(ctx, Control_TF_IsValid(tf_ptr), code, msg, CCONTROL_EMPTY_TF)

void Control_System_Init(ControlHandle *ctx, ControlArena *p, ControlArena *s)
{
    ctx->persistent = p;
    ctx->scratch = s;
}

void Control_System_DeInit(ControlHandle *ctx)
{
    Control_Arena_Clear(ctx->scratch);
    Control_Arena_Clear(ctx->persistent);
}

CONTROL_PRIVATE_API ControlVec __Control_Vec_CreateInArena(ControlArena *a, size_t capacity)
{
    ControlVec v;
    v.capacity = 0;
    v.size = 0;
    v.coeffs = NULL;

    v.coeffs = (float *)Control_Arena_Alloc(a, capacity * sizeof(float));
    if (!Control_Vec_IsValid(&v))
    {
        return CCONTROL_EMPTY_VEC;
    }

    v.capacity = capacity;
    return v;
}

static ControlVec __Control_Poly_CreateInArena(ControlArena *a, const float *coeffs, size_t size)
{
    if (coeffs == NULL || size <= 0)
    {
        return CCONTROL_EMPTY_VEC;
    }

    ControlVec v = __Control_Vec_CreateInArena(a, size);
    if (!Control_Vec_IsValid(&v))
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

ControlVec Control_Poly_Canonicalize(const ControlVec *v)
{
    ControlVec canon = *v;

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
ControlVec Control_Poly_AllocScratch(ControlHandle *ctx, const float *coeffs, size_t size)
{
    ControlVec v = __Control_Poly_CreateInArena(ctx->scratch, coeffs, size);
    REQUIRE_VALID_VEC(ctx, &v, "Could not allocate in scratch arena");

    return v;
}

ControlVec Control_Poly_AllocPersistent(ControlHandle *ctx, const float *coeffs, size_t size)
{
    ControlVec v = __Control_Poly_CreateInArena(ctx->persistent, coeffs, size);
    REQUIRE_VALID_VEC(ctx, &v, "Could not allocate in persistent arena");

    return v;
}

static ControlVec
__Control_Poly_AddInArena(ControlArena *arena, const ControlVec *a, const ControlVec *b)
{
    size_t max_size = a->size > b->size ? a->size : b->size;
    ControlVec vec = __Control_Vec_CreateInArena(arena, max_size);
    if (!Control_Vec_IsValid(&vec))
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

ControlVec Control_Poly_Add(ControlHandle *ctx, const ControlVec *a, const ControlVec *b)
{
    ControlVec v = __Control_Poly_AddInArena(ctx->scratch, a, b);
    REQUIRE_VALID_VEC(ctx, &v, "Could not create vector for polynomial addition in scratch arena");

    return v;
}

static ControlVec __Control_Poly_MultiplyInArena(ControlArena *arena, ControlVec *a, ControlVec *b)
{
    if (!Control_Vec_IsValid(a) || !Control_Vec_IsValid(b))
    {
        return CCONTROL_EMPTY_VEC;
    }
    size_t new_size = a->size + b->size - 1;

    ControlVec result = __Control_Vec_CreateInArena(arena, new_size);
    if (!Control_Vec_IsValid(&result))
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

ControlVec Control_Poly_Multiply(ControlHandle *ctx, ControlVec *a, ControlVec *b)
{
    ControlVec v = __Control_Poly_MultiplyInArena(ctx->scratch, a, b);
    REQUIRE_VALID_VEC(ctx, &v, "Could not multiply polynomials");

    return v;
}

ControlTransferFunction Control_TF_FromPoly(const ControlVec *num, const ControlVec *dem)
{
    ControlTransferFunction G = {
        .num = *num,
        .dem = *dem,
    };
    return G;
}

static ControlTransferFunction __Control_TF_MultiplyInArena(ControlArena *a,
                                                            ControlTransferFunction *G1,
                                                            ControlTransferFunction *G2)
{
    ControlVec conv_num = __Control_Poly_MultiplyInArena(a, &G1->num, &G2->num);
    if (!Control_Vec_IsValid(&conv_num))
    {
        return CCONTROL_EMPTY_TF;
    }

    ControlVec conv_dem = __Control_Poly_MultiplyInArena(a, &G1->dem, &G2->dem);
    if (!Control_Vec_IsValid(&conv_dem))
    {
        return CCONTROL_EMPTY_TF;
    }

    ControlVec clean_num = Control_Poly_Canonicalize(&conv_num);
    ControlVec clean_dem = Control_Poly_Canonicalize(&conv_dem);

    return Control_TF_FromPoly(&clean_num, &clean_dem);
}

ControlTransferFunction
Control_TF_Multiply(ControlHandle *ctx, ControlTransferFunction *G1, ControlTransferFunction *G2)
{
    REQUIRE_VALID_TF(
        ctx, G1, CCONTROL_ERROR_INVALID_ARGUMENT, "TransferFunction G1 is NULL or invalid");
    REQUIRE_VALID_TF(
        ctx, G2, CCONTROL_ERROR_INVALID_ARGUMENT, "TransferFunction G2 is NULL or invalid");

    ControlTransferFunction tf = __Control_TF_MultiplyInArena(ctx->scratch, G1, G2);
    REQUIRE_VALID_TF(
        ctx, &tf, CCONTROL_ERROR_OUT_OF_MEMORY, "Could not multiply transfer function");
    return tf;
}

inline bool Control_TF_IsValid(ControlTransferFunction *tf)
{
    return tf != NULL && tf->num.coeffs != NULL && tf->dem.coeffs != NULL;
}

ControlTransferFunction Control_TF_ClosedLoop(ControlHandle *ctx,
                                              ControlTransferFunction *G,
                                              float gain,
                                              ControlFeedbackUnity unity)
{
    REQUIRE_VALID_TF(
        ctx, G, CCONTROL_ERROR_DIVIDE_BY_ZERO, "TransferFunction G is NULL or invalid");
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

    ControlVec scaled_num = {0};
    if (gain != 1.0f)
    {
        scaled_num = __Control_Vec_CreateInArena(ctx->scratch, G->num.size);
        CCONTROL_REQUIRE(ctx,
                         Control_Vec_IsValid(&scaled_num),
                         CCONTROL_ERROR_OUT_OF_MEMORY,
                         "Failed to allocate vector in scratch arena",
                         CCONTROL_EMPTY_TF);

        for (size_t i = 0; i < G->num.size; i++)
        {
            scaled_num.coeffs[i] = G->num.coeffs[i] * gain;
        }
    }
    else
    {
        scaled_num = G->num;
    }

    ControlVec denom = __Control_Poly_AddInArena(ctx->scratch, &G->dem, &scaled_num);
    if (!Control_Vec_IsValid(&denom))
    {
        CCONTROL_THROW(ctx,
                       CCONTROL_ERROR_OUT_OF_MEMORY,
                       "Could not allocate vector for polynomial addition for "
                       "closed loop");
        return CCONTROL_EMPTY_TF;
    }

    return Control_TF_FromPoly(&G->num, &denom);
}

ControlTransferFunction Control_TF_Persist(ControlHandle *ctx, const ControlTransferFunction *tf)
{
    ControlTransferFunction p_tf = {0};
    CCONTROL_REQUIRE(
        ctx, tf != NULL, CCONTROL_ERROR_INVALID_ARGUMENT, "Invalid TF", CCONTROL_EMPTY_TF);

    p_tf.num = __Control_Poly_CreateInArena(ctx->persistent, tf->num.coeffs, tf->num.size);
    p_tf.dem = __Control_Poly_CreateInArena(ctx->persistent, tf->dem.coeffs, tf->dem.size);

    return p_tf;
}
