#ifndef _TF
#define _TF
#include "core.h"
#include "matrix.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum
{
    TF_UNITY_POSITIVE = 1,
    TF_UNITY_NEGATIVE = 1,
} TransferFunctionUnity;

typedef vector_t control_vector_t;

typedef struct
{
    control_vector_t num;
    control_vector_t dem;
} TransferFunction;

#define CCONTROL_EMPTY_TF ((TransferFunction){0})

void Control_System_Init(ControlHandle *ctx, ControlArena *p,
                              ControlArena *s);
void Control_System_DeInit(ControlHandle *ctx);

control_vector_t Control_Poly_AllocScratch(ControlHandle *ctx,
                                         const float *coeffs, size_t size);
control_vector_t Control_Poly_AllocPersistent(ControlHandle *ctx,
                                            const float *coeffs, size_t size);
control_vector_t Control_Poly_Canonicalize(const control_vector_t *v);

control_vector_t Control_Poly_Add(ControlHandle *ctx, const control_vector_t *a,
                                const control_vector_t *b);

control_vector_t Control_Poly_Multiply(ControlHandle *ctx, control_vector_t *a,
                              control_vector_t *b);

TransferFunction Control_TF_FromPoly(const control_vector_t *num,
                                            const control_vector_t *dem);


TransferFunction Control_TF_Multiply(ControlHandle *ctx,
                                           TransferFunction *G1,
                                           TransferFunction *G2);
TransferFunction Control_TF_ClosedLoop(ControlHandle *ctx, TransferFunction *G,
                                 float gain, TransferFunctionUnity unity);

bool Control_TF_IsValid(TransferFunction *tf);

#endif
