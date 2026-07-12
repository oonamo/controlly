#include <ccontrol/matrix.h>

vector_t Control_Vec_Alloc(ControlArena *a, size_t size)
{
    vector_t v;
    v.capacity = size;
    v.size = 0;

    v.coeffs = (float *)Control_Arena_Alloc(a, size * sizeof(float));
    if (!v.coeffs)
        return CCONTROL_EMPTY_VEC;

    for (size_t i = 0; i < size; i++)
    {
        v.coeffs[i] = 0.0f;
    }
    return v;
}

matrix_t Control_Matrix_Alloc(ControlArena *a, size_t rows, size_t cols)
{
    matrix_t m;

    m.data = (float *)Control_Arena_Alloc(a, rows * cols * sizeof(float));
    if (!m.data)
        return CCONTROL_EMPTY_MATRIX;

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

vector_t _CreateVectorInArena(ControlArena *a, size_t capacity)
{
    vector_t v;
    v.capacity = capacity;
    v.size = 0;

    v.coeffs = (float *)Control_Arena_Alloc(a, capacity * sizeof(float));
    return v;
}

vector_t Control_Matrix_MultiplyVec(ControlArena *a, const matrix_t *m, const vector_t *v)
{
    if (!m || !v || m->cols != v->size)
    {
        return CCONTROL_EMPTY_VEC;
    }
    size_t new_size = m->rows;

    vector_t res = _CreateVectorInArena(a, new_size);
    if (Control_Vec_IsValid(&res))
        return res;

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

vector_t Control_Vec_Add(ControlArena *a, vector_t *lhs, vector_t *rhs)
{
    size_t max_size = lhs->size > rhs->size ? lhs->size : rhs->size;

    vector_t res = _CreateVectorInArena(a, max_size);
    if (!Control_Vec_IsValid(&res))
        return res;

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

vector_t Control_Vec_Scale(ControlArena *a, vector_t *v, float scalar)
{
    vector_t res = _CreateVectorInArena(a, v->size);
    if (!Control_Vec_IsValid(&res))
        return CCONTROL_EMPTY_VEC;

    res.size = v->size;

    for (size_t i = 0; i < v->size; i++)
    {
        res.coeffs[i] *= scalar;
    }

    return res;
}

inline bool Control_Matrix_IsValid(matrix_t *m) { return m != NULL && m->data != NULL; }
inline bool Control_Vec_IsValid(vector_t *v)
{
    return v != NULL && v->coeffs != NULL;
}
