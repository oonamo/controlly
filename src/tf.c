#include "internal_common.h"
#include <ccontrol/arena.h>
#include <ccontrol/core.h>
#include <ccontrol/matrix.h>
#include <ccontrol/tf.h>
#include <stddef.h>

#define REQUIRE_VALID_TF(ctx, tf_ptr, msg)                                                         \
    CCONTROL_REQUIRE(ctx, Control_TF_IsValid(tf_ptr), CCONTROL_ERROR_INVALID_ARGUMENT, msg)

CONTROL_PRIVATE_API ControlResult __Control_Vec_CreateInArena(ControlHandle *ctx,
                                                              ControlVec    *out,
                                                              ControlArena  *a,
                                                              size_t         capacity)
{
    out->capacity = 0;
    out->size     = 0;
    out->coeffs   = NULL;

    out->coeffs = (float *)Control_Arena_Alloc(a, capacity * sizeof(float));
    CCONTROL_REQUIRE(ctx, out->coeffs, CCONTROL_ERROR_OUT_OF_MEMORY, "out of memory");

    out->capacity = capacity;
    return CCONTROL_OK;
}

static ControlResult __Control_Poly_CreateInArena(
    ControlHandle *ctx, ControlVec *out, ControlArena *a, const float *coeffs, size_t size)
{
    CCONTROL_TRY(__Control_Vec_CreateInArena(ctx, out, a, size));

    out->size = size;

    for (size_t i = 0; i < size; i++)
    {
        out->coeffs[i] = coeffs[i];
    }

    return CCONTROL_OK;
}

ControlResult Control_Poly_Canonicalize(ControlHandle *ctx, ControlVec *out, const ControlVec *v)
{
    CHECK_CTX(ctx);
    CHECK_NOT_NULL(ctx, out && v, "Null pointer was passed");

    size_t i = 0;

    // TODO: Compare with epsilon
    while (i + 1 < v->size && v->coeffs[i] == 0.0f)
    {
        i++;
    }

    size_t new_capacity = v->capacity - i;

    out->coeffs   = &v->coeffs[i];
    out->size     = v->size - i;
    out->capacity = new_capacity;

    return CCONTROL_OK;
}

ControlResult
Control_Poly_AllocScratch(ControlHandle *ctx, ControlVec *out, const float *coeffs, size_t size)
{
    CHECK_CTX(ctx);
    CHECK_NOT_NULL(ctx, out && coeffs, "Null pointer was passed");

    return __Control_Poly_CreateInArena(ctx, out, ctx->scratch, coeffs, size);
}

ControlResult
Control_Poly_AllocPersistent(ControlHandle *ctx, ControlVec *out, const float *coeffs, size_t size)
{
    CHECK_CTX(ctx);
    CHECK_NOT_NULL(ctx, out && coeffs, "Null pointer was passed");

    return __Control_Poly_CreateInArena(ctx, out, ctx->persistent, coeffs, size);
}

ControlResult
Control_Poly_Add(ControlHandle *ctx, ControlVec *out, const ControlVec *a, const ControlVec *b)
{
    CHECK_CTX(ctx);
    CHECK_NOT_NULL(ctx, out && a && b, "Null pointer was passed");

    size_t max_size = a->size > b->size ? a->size : b->size;

    if (out->coeffs == NULL || out->capacity < max_size)
    {
        CCONTROL_TRY(__Control_Vec_CreateInArena(ctx, out, ctx->scratch, max_size));
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

        out->coeffs[i] = sum;
    }

    out->size = max_size;
    return CCONTROL_OK;
}

ControlResult
Control_Poly_Multiply(ControlHandle *ctx, ControlVec *out, const ControlVec *a, const ControlVec *b)
{
    CHECK_CTX(ctx);
    CHECK_NOT_NULL(ctx, out && a && b, "Null pointer was passed");

    size_t new_size = a->size + b->size - 1;
    if (out->coeffs == NULL || out->capacity < new_size)
    {
        CCONTROL_TRY(__Control_Vec_CreateInArena(ctx, out, ctx->scratch, new_size));
    }

    out->size = new_size;

    for (size_t i = 0; i < new_size; i++)
    {
        out->coeffs[i] = 0.0f;
    }

    for (size_t i = 0; i < a->size; i++)
    {
        for (size_t j = 0; j < b->size; j++)
        {
            out->coeffs[i + j] += a->coeffs[i] * b->coeffs[j];
        }
    }

    return CCONTROL_OK;
}

ControlResult Control_TF_FromPoly(ControlHandle           *ctx,
                                  ControlTransferFunction *out,
                                  const ControlVec        *num,
                                  const ControlVec        *dem)
{
    CHECK_CTX(ctx);
    CHECK_NOT_NULL(ctx, out && num && dem, "Out pointer is NULL");

    // TODO: Check if we can use allocated memory if abailable
    out->num = *num;
    out->den = *dem;
    return CCONTROL_OK;
}

ControlResult Control_TF_Multiply(ControlHandle                 *ctx,
                                  ControlTransferFunction       *out,
                                  const ControlTransferFunction *G1,
                                  const ControlTransferFunction *G2)
{
    CHECK_CTX(ctx);
    CHECK_NOT_NULL(ctx, out, "Out pointer is NULL");

    CCONTROL_TRY(Control_TF_Validate(ctx, G1));
    CCONTROL_TRY(Control_TF_Validate(ctx, G2));

    ControlVec convolved_num = {0};
    ControlVec convolved_den = {0};

    CCONTROL_TRY(Control_Poly_Multiply(ctx, &convolved_num, &G1->num, &G2->num));
    CCONTROL_TRY(Control_Poly_Multiply(ctx, &convolved_den, &G1->den, &G2->den));

    CCONTROL_TRY(Control_Poly_Canonicalize(ctx, &convolved_num, &convolved_num));
    CCONTROL_TRY(Control_Poly_Canonicalize(ctx, &convolved_den, &convolved_den));

    CCONTROL_TRY(Control_TF_FromPoly(ctx, out, &convolved_num, &convolved_den));

    return CCONTROL_OK;
}

inline bool Control_TF_IsValid(const ControlTransferFunction *tf)
{
    return tf && tf->den.coeffs && tf->num.coeffs && tf->den.size != 0 && tf->den.coeffs[0] != 0;
}

ControlResult Control_TF_Validate(ControlHandle *ctx, const ControlTransferFunction *tf)
{
    CHECK_CTX(ctx);
    CHECK_NOT_NULL(ctx, tf, "Transfer function pointer is NULL");

    CCONTROL_REQUIRE(ctx,
                     tf->num.coeffs && tf->den.coeffs,
                     CCONTROL_ERROR_INVALID_ARGUMENT,
                     "Transfer function pointer is null");

    CCONTROL_REQUIRE(ctx,
                     tf->den.size != 0 && tf->den.coeffs[0] != 0.0f,
                     CCONTROL_ERROR_DIVIDE_BY_ZERO,
                     "Transfer function denominator is zero");

    return CCONTROL_OK;
}

ControlResult Control_TF_ClosedLoop(ControlHandle                 *ctx,
                                    ControlTransferFunction       *out,
                                    const ControlTransferFunction *G,
                                    float                          gain,
                                    ControlFeedbackType            unity)
{
    CHECK_CTX(ctx);
    CHECK_NOT_NULL(ctx, out, "Null pointer was passed");
    CCONTROL_TRY(Control_TF_Validate(ctx, G));

    /*
     * G(s) = N(s)/D(s)
     *
     * H(s) = G(s)/(1 +/- k*G(s)) = (N(s)/D(s))/(1 +/- k*(N(s)/D(s))
     * H(s) = N(s)/(D(s) +/- k*N(s))
     */

    if (unity == TF_FEEDBACK_POSITIVE)
    {
        gain = -gain;
    }

    ControlVec scaled_num = {0};
    if (gain != 1.0f)
    {
        CCONTROL_TRY(__Control_Vec_CreateInArena(ctx, &scaled_num, ctx->scratch, G->num.size));
        for (size_t i = 0; i < G->num.size; i++)
        {
            scaled_num.coeffs[i] = G->num.coeffs[i] * gain;
        }
    }
    else
    {
        scaled_num = G->num;
    }

    ControlVec denom = {0};
    CCONTROL_TRY(Control_Poly_Add(ctx, &denom, &G->den, &scaled_num));
    CCONTROL_TRY(Control_TF_FromPoly(ctx, out, &G->num, &denom));

    return CCONTROL_OK;
}

ControlResult Control_TF_Persist(ControlHandle                 *ctx,
                                 ControlTransferFunction       *out,
                                 const ControlTransferFunction *tf)
{
    CHECK_CTX(ctx);
    CHECK_NOT_NULL(ctx, out, "Out parameter was NULL");

    // NOTE: We do not validate, invalid state can be copied
    // CCONTROL_TRY(Control_TF_Validate(ctx, tf));

    CCONTROL_TRY(__Control_Poly_CreateInArena(
        ctx, &out->num, ctx->persistent, tf->num.coeffs, tf->num.size));
    CCONTROL_TRY(__Control_Poly_CreateInArena(
        ctx, &out->den, ctx->persistent, tf->den.coeffs, tf->den.size));

    return CCONTROL_OK;
}
