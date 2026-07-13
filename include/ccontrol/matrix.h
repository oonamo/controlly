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
} ControlVec;

#define CCONTROL_EMPTY_VEC ((ControlVec){0})

typedef struct
{
    size_t rows;
    size_t cols;
    float *data;
} ControlMatrix;

#define CCONTROL_EMPTY_MATRIX ((ControlMatrix){0})

ControlVec Control_Vec_Alloc(ControlArena *a, size_t size);
ControlMatrix Control_Matrix_Alloc(ControlArena *a, size_t rows, size_t cols);
ControlVec Control_Matrix_MultiplyVec(ControlArena *a, const ControlMatrix *m, const ControlVec *v);
ControlVec Control_Vec_Add(ControlArena *a, ControlVec *lhs, ControlVec *rhs);
ControlVec Control_Vec_Scale(ControlArena *a, ControlVec *v, float scalar);

bool Control_Matrix_IsValid(ControlMatrix *m);
bool Control_Vec_IsValid(ControlVec *v);
#endif
