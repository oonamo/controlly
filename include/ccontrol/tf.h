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
} ControlFeedbackUnity;

typedef struct
{
    ControlVec num;
    ControlVec dem;
} ControlTransferFunction;

#define CCONTROL_EMPTY_TF ((ControlTransferFunction){0})

ControlVec Control_Poly_AllocScratch(ControlHandle *ctx, const float *coeffs, size_t size);
ControlVec Control_Poly_AllocPersistent(ControlHandle *ctx, const float *coeffs, size_t size);
ControlVec Control_Poly_Canonicalize(const ControlVec *v);

ControlVec Control_Poly_Add(ControlHandle *ctx, const ControlVec *a, const ControlVec *b);

ControlVec Control_Poly_Multiply(ControlHandle *ctx, ControlVec *a, ControlVec *b);

ControlTransferFunction Control_TF_FromPoly(const ControlVec *num, const ControlVec *dem);

ControlTransferFunction
Control_TF_Multiply(ControlHandle *ctx, ControlTransferFunction *G1, ControlTransferFunction *G2);

ControlTransferFunction Control_TF_ClosedLoop(ControlHandle *ctx,
                                              ControlTransferFunction *G,
                                              float gain,
                                              ControlFeedbackUnity unity);

bool Control_TF_IsValid(ControlTransferFunction *tf);

#endif
