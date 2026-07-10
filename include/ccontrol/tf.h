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

void ControlSystem_InitHandle(ControlHandle *ctx, ControlArena *p,
                              ControlArena *s);
void ControlSystem_DeInitHandle(ControlHandle *ctx);

control_vector_t PolyCoeffVector_Scratch(ControlHandle *ctx,
                                         const float *coeffs, size_t size);
control_vector_t PolyCoeffVector_Persistent(ControlHandle *ctx,
                                            const float *coeffs, size_t size);
control_vector_t PolyCoeffVector_Cannonicalize(const control_vector_t *v);
int PolyCoeffVectorToStr(const control_vector_t *coeffs, char *buffer,
                         size_t buffer_size);

control_vector_t AddCoeffVector(ControlHandle *ctx, const control_vector_t *a,
                                const control_vector_t *b);

TransferFunction TransferFunctionFromCoeffs(const control_vector_t *num,
                                            const control_vector_t *dem);
int TransferFunctionToStr(TransferFunction *tf, char *buffer,
                          size_t buffer_size);

control_vector_t MultiplyPoly(ControlHandle *ctx, control_vector_t *a,
                              control_vector_t *b);

TransferFunction MultiplyTransferFunctions(ControlHandle *ctx,
                                           TransferFunction *G1,
                                           TransferFunction *G2);
TransferFunction UnityClosedLoop(ControlHandle *ctx, TransferFunction *G,
                                 float gain, TransferFunctionUnity unity);

bool TransferFunction_IsValid(TransferFunction *tf);

#endif
