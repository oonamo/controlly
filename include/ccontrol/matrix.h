#ifndef _MATRIX_H
#define _MATRIX_H
#include "arena.h"
#include <stddef.h>

typedef struct
{
    size_t size;
    size_t capacity;
    float *coeffs;
} vector_t;

typedef struct
{
    size_t rows;
    size_t cols;
    float *data;
} matrix_t;

typedef matrix_t system_matrix_t;
typedef vector_t input_matrix_t;
typedef vector_t output_matrix_t;
typedef vector_t feedback_matrix_t;

vector_t ArenaAllocVec(ControlArena* a, size_t size);
matrix_t ArenaAllocMatrix(ControlArena* a, size_t rows, size_t cols);
vector_t VectorMulMatrix(ControlArena *a, const matrix_t *m, const vector_t *v);
vector_t VectorAdd(ControlArena *a, vector_t *lhs, vector_t *rhs);
vector_t VectorScalar(ControlArena *a, vector_t *v, float scalar);
#endif
