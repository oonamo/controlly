#ifndef _MATRIX_H
#define _MATRIX_H
#include <ccontrol/core.h>
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
#define CCONTOL_INIT_VEC      {0}

ControlResult Control_Vec_AllocScratch(ControlHandle *ctx, ControlVec *out, size_t size);
ControlResult Control_Vec_AllocPersistent(ControlHandle *ctx, ControlVec *out, size_t size);

ControlResult Control_Vec_Persist(ControlHandle *ctx, ControlVec *out, const ControlVec *v);
ControlResult
Control_Matrix_AllocPersistent(ControlHandle *ctx, ControlMatrix *out, size_t rows, size_t cols);

ControlResult
Control_Matrix_AllocScratch(ControlHandle *ctx, ControlMatrix *out, size_t rows, size_t cols);

ControlResult
Control_Matrix_Persist(ControlHandle *ctx, ControlMatrix *out, const ControlMatrix *m);
ControlResult Control_Matrix_MultiplyVec(ControlHandle *ctx,
                                         ControlVec *out,
                                         const ControlMatrix *m,
                                         const ControlVec *v);
ControlResult
Control_Vec_Add(ControlHandle *ctx, ControlVec *out, const ControlVec *lhs, const ControlVec *rhs);
ControlResult Control_Vec_Scale(ControlHandle *ctx, ControlVec *out, const ControlVec *v, float scalar);

bool Control_Matrix_IsValid(const ControlMatrix *m);
bool Control_Vec_IsValid(const ControlVec *v);
#endif
