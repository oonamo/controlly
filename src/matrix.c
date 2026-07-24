#include "internal_common.h"
#include <ccontrol/core.h>
#include <ccontrol/matrix.h>

// TODO: Add testing
//  1. Does it currently support aliasing?

static ControlResult
__Control_Vec_AllocInArena(ControlHandle *ctx, ControlVec *out, ControlArena *a, size_t size)
{

    out->size   = 0;
    out->coeffs = (float *)Control_Arena_Alloc(a, size * sizeof(float));
    CCONTROL_REQUIRE(ctx, out->coeffs, CCONTROL_ERROR_OUT_OF_MEMORY, "out of memory");

    out->capacity = size;

    // TODO: Does this need to be reset?
    for (size_t i = 0; i < size; i++)
    {
        out->coeffs[i] = 0.0f;
    }
    return CCONTROL_OK;
}

ControlResult Control_Vec_AllocScratch(ControlHandle *ctx, ControlVec *out, size_t size)
{
    CHECK_CTX(ctx);
    CHECK_OUT(ctx, out);

    CCONTROL_TRY(__Control_Vec_AllocInArena(ctx, out, ctx->scratch, size));
    return CCONTROL_OK;
}
ControlResult Control_Vec_AllocPersistent(ControlHandle *ctx, ControlVec *out, size_t size)

{
    CHECK_CTX(ctx);
    CHECK_OUT(ctx, out);

    CCONTROL_TRY(__Control_Vec_AllocInArena(ctx, out, ctx->persistent, size));
    return CCONTROL_OK;
}

ControlResult Control_Vec_Persist(ControlHandle *ctx, ControlVec *out, const ControlVec *v)
{
    CHECK_CTX(ctx);
    CHECK_OUT(ctx, out);

    if (out->capacity < v->size)
    {
        CCONTROL_TRY(Control_Vec_AllocPersistent(ctx, out, v->size));
    }

    out->size = v->size;

    // TODO: Abstract into some sort of memcpy (either from ControlHandle or macro)
    for (size_t i = 0; i < v->size; i++)
    {
        out->coeffs[i] = v->coeffs[i];
    }

    return CCONTROL_OK;
}

ControlResult __Control_Matrix_Alloc(
    ControlHandle *ctx, ControlMatrix *out, ControlArena *a, size_t rows, size_t cols)
{
    out->data = (float *)Control_Arena_Alloc(a, rows * cols * sizeof(float));
    CCONTROL_REQUIRE(ctx, out->data, CCONTROL_ERROR_OUT_OF_MEMORY, "out of memory");

    // TODO: Does this need to be set to 0.0f?
    for (size_t i = 0; i < rows; i++)
    {
        for (size_t j = 0; j < cols; j++)
        {
            out->data[i * cols + j] = 0.0f;
        }
    }

    out->rows = rows;
    out->cols = cols;
    return CCONTROL_OK;
}

ControlResult
Control_Matrix_AllocScratch(ControlHandle *ctx, ControlMatrix *out, size_t rows, size_t cols)
{
    CHECK_CTX(ctx);
    CHECK_OUT(ctx, out);

    CCONTROL_TRY(__Control_Matrix_Alloc(ctx, out, ctx->scratch, rows, cols));
    return CCONTROL_OK;
}

ControlResult
Control_Matrix_AllocPersistent(ControlHandle *ctx, ControlMatrix *out, size_t rows, size_t cols)
{
    CHECK_CTX(ctx);
    CHECK_OUT(ctx, out);

    CCONTROL_TRY(__Control_Matrix_Alloc(ctx, out, ctx->persistent, rows, cols));
    return CCONTROL_OK;
}

ControlResult Control_Matrix_Persist(ControlHandle *ctx, ControlMatrix *out, const ControlMatrix *m)
{
    CHECK_CTX(ctx);
    CHECK_OUT(ctx, out);

    if (out->rows != m->rows || out->cols != m->cols)
    {
        CCONTROL_TRY(Control_Matrix_AllocPersistent(ctx, out, m->rows, m->cols));
    }

    // TODO: Abstract into CCONTROL_MEMCPY
    for (size_t i = 0; i < m->rows; i++)
    {
        for (size_t j = 0; j < m->cols; j++)
        {
            out->data[i * m->cols + j] = m->data[i * m->cols + j];
        }
    }

    return CCONTROL_OK;
}

ControlResult __Control_Vec_CreateInArenaRaw(ControlHandle *ctx,
                                             ControlVec    *out,
                                             ControlArena  *a,
                                             size_t         capacity)
{
    out->size   = 0;
    out->coeffs = (float *)Control_Arena_Alloc(a, capacity * sizeof(float));
    CCONTROL_REQUIRE(ctx, out->coeffs, CCONTROL_ERROR_OUT_OF_MEMORY, "Out of memory");

    out->capacity = capacity;
    return CCONTROL_OK;
}

ControlResult Control_Matrix_MultiplyVec(ControlHandle       *ctx,
                                         ControlVec          *out,
                                         const ControlMatrix *m,
                                         const ControlVec    *v)
{
    CHECK_CTX(ctx);
    CHECK_NOT_NULL(ctx, out && m && v, "Passed null pointers");
    CCONTROL_REQUIRE(
        ctx, m->cols == v->size, CCONTROL_ERROR_DIMENSION_MISMATCH, "Dimension mismatch");

    size_t new_size = m->rows;

    ControlVec res = {0};
    CCONTROL_TRY(__Control_Vec_CreateInArenaRaw(ctx, &res, ctx->scratch, new_size));

    res.size = new_size;

    for (size_t i = 0; i < new_size; i++)
    {
        float dot_prod = 0.0f;
        for (size_t j = 0; j < m->cols; j++)
        {
            dot_prod += m->data[i * m->cols + j] * v->coeffs[j];
        }

        res.coeffs[i] = dot_prod;
    }

    if (out->capacity < new_size)
    {
        CCONTROL_TRY(__Control_Vec_CreateInArenaRaw(ctx, out, ctx->scratch, new_size));
    }

    out->size = new_size;
    for (size_t i = 0; i < new_size; i++)
    {
        out->coeffs[i] = res.coeffs[i];
    }

    return CCONTROL_OK;
}

ControlResult
Control_Vec_Add(ControlHandle *ctx, ControlVec *out, const ControlVec *lhs, const ControlVec *rhs)
{
    CHECK_CTX(ctx);
    CHECK_NOT_NULL(ctx, out && lhs && rhs, "Passed null pointers");
    CCONTROL_REQUIRE(
        ctx, lhs->size == rhs->size, CCONTROL_ERROR_DIMENSION_MISMATCH, "dimension mismatch");

    size_t size = lhs->size;
    if (out->capacity < size)
    {
        CCONTROL_TRY(__Control_Vec_CreateInArenaRaw(ctx, out, ctx->scratch, size));
    }

    out->size = size;

    for (size_t i = 0; i < size; i++)
    {
        float sum = 0.0f;
        if (i < lhs->size)
        {
            sum += lhs->coeffs[i];
        }
        if (i < rhs->size)
        {
            sum += rhs->coeffs[i];
        }

        out->coeffs[i] = sum;
    }

    return CCONTROL_OK;
}

ControlResult
Control_Vec_Scale(ControlHandle *ctx, ControlVec *out, const ControlVec *v, float scalar)
{
    CHECK_CTX(ctx);
    CHECK_NOT_NULL(ctx, out && v, "Passed null pointers");

    if (out->capacity < v->size)
    {
        CCONTROL_TRY(__Control_Vec_CreateInArenaRaw(ctx, out, ctx->scratch, v->size));
    }

    out->size = v->size;

    for (size_t i = 0; i < v->size; i++)
    {
        out->coeffs[i] = v->coeffs[i] * scalar;
    }

    return CCONTROL_OK;
}

bool Control_Matrix_IsValid(const ControlMatrix *m)
{
    return m != NULL && m->data != NULL;
}

bool Control_Vec_IsValid(const ControlVec *v)
{
    return v != NULL && v->coeffs != NULL;
}
