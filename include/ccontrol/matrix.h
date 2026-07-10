#ifndef _MATRIX_H
#define _MATRIX_H
#include "arena.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct
{
    size_t size;
    size_t capacity;
    float *coeffs;
} vector_t;

#define CCONTROL_EMPTY_VEC ((vector_t){0})

typedef struct
{
    size_t rows;
    size_t cols;
    float *data;
} matrix_t;

#define CCONTROL_EMPTY_MATRIX ((matrix_t){0})


vector_t ArenaAllocVec(ControlArena *a, size_t size);
matrix_t ArenaAllocMatrix(ControlArena *a, size_t rows, size_t cols);
vector_t VectorMulMatrix(ControlArena *a, const matrix_t *m, const vector_t *v);
vector_t VectorAdd(ControlArena *a, vector_t *lhs, vector_t *rhs);
vector_t VectorScalar(ControlArena *a, vector_t *v, float scalar);

bool Matrix_IsValid(matrix_t *m);
bool Vector_IsValid(vector_t *v);
#endif
