#include "ccontrol/arena.h"
#include <ccontrol/matrix.h>

ControlVec Control_Vec_Alloc(ControlArena *a, size_t size)
{
    ControlVec v;
    v.capacity = size;
    v.size = 0;

    v.coeffs = (float *)Control_Arena_Alloc(a, size * sizeof(float));
    if (!v.coeffs)
    {
        return CCONTROL_EMPTY_VEC;
    }

    // TODO: Does this need to be reset?
    for (size_t i = 0; i < size; i++)
    {
        v.coeffs[i] = 0.0f;
    }
    return v;
}

ControlVec Control_Vec_Clone(ControlArena *a, const ControlVec *v)
{
    ControlVec cloned = Control_Vec_Alloc(a, v->size);
    if (!Control_Vec_IsValid(&cloned))
    {
        return CCONTROL_EMPTY_VEC;
    }

    cloned.size = v->size;

    // TODO: Abstract into CCONTROL_MEMCPY
    for (size_t i = 0; i < v->size; i++)
    {
        cloned.coeffs[i] = v->coeffs[i];
    }

    return cloned;
}

ControlMatrix Control_Matrix_Alloc(ControlArena *a, size_t rows, size_t cols)
{
    ControlMatrix m = {0};

    m.data = (float *)Control_Arena_Alloc(a, rows * cols * sizeof(float));
    if (!m.data)
    {
        return CCONTROL_EMPTY_MATRIX;
    }

    // TODO: Does this need to be set to 0.0f?
    for (size_t i = 0; i < rows; i++)
    {
        for (size_t j = 0; j < cols; j++)
        {
            m.data[i * cols + j] = 0.0f;
        }
    }

    m.rows = rows;
    m.cols = cols;
    return m;
}

ControlMatrix Control_Matrix_Clone(ControlArena *a, const ControlMatrix *m)
{
    ControlMatrix cloned = Control_Matrix_Alloc(a, m->rows, m->cols);
    if (!Control_Matrix_IsValid(&cloned))
    {
        return CCONTROL_EMPTY_MATRIX;
    }

    // TODO: Abstract into CCONTROL_MEMCPY
    for (size_t i = 0; i < m->rows; i++)
    {
        for (size_t j = 0; j < m->cols; j++)
        {
            cloned.data[i * m->cols + j] = m->data[i * m->cols + j];
        }
    }

    return cloned;
}

ControlVec _CreateVectorInArena(ControlArena *a, size_t capacity)
{
    ControlVec v;
    v.capacity = capacity;
    v.size = 0;

    v.coeffs = (float *)Control_Arena_Alloc(a, capacity * sizeof(float));
    return v;
}

ControlVec Control_Matrix_MultiplyVec(ControlArena *a, const ControlMatrix *m, const ControlVec *v)
{
    if (!m || !v || m->cols != v->size)
    {
        return CCONTROL_EMPTY_VEC;
    }
    size_t new_size = m->rows;

    ControlVec res = _CreateVectorInArena(a, new_size);
    if (Control_Vec_IsValid(&res))
    {
        return res;
    }

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

    return res;
}

ControlVec Control_Vec_Add(ControlArena *a, ControlVec *lhs, ControlVec *rhs)
{
    size_t max_size = lhs->size > rhs->size ? lhs->size : rhs->size;

    ControlVec res = _CreateVectorInArena(a, max_size);
    if (!Control_Vec_IsValid(&res))
    {
        return res;
    }

    res.size = max_size;

    for (size_t i = 0; i < max_size; i++)
    {
        float sum = 0.0f;
        if (i < lhs->size - 1)
        {
            sum += lhs->coeffs[i];
        }
        if (i < rhs->size - 1)
        {
            sum += rhs->coeffs[i];
        }

        res.coeffs[i] = sum;
    }

    return res;
}

ControlVec Control_Vec_Scale(ControlArena *a, ControlVec *v, float scalar)
{
    ControlVec res = _CreateVectorInArena(a, v->size);

    if (!Control_Vec_IsValid(&res))
    {
        return CCONTROL_EMPTY_VEC;
    }

    res.size = v->size;

    for (size_t i = 0; i < v->size; i++)
    {
        res.coeffs[i] *= scalar;
    }

    return res;
}

inline bool Control_Matrix_IsValid(ControlMatrix *m)
{
    return m != NULL && m->data != NULL;
}
inline bool Control_Vec_IsValid(ControlVec *v)
{
    return v != NULL && v->coeffs != NULL;
}
