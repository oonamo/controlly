#include "arena.h"
#include "matrix.h"
#include <stdint.h>

typedef struct
{
    ControlArena *persistent;
    ControlArena *scratch;
} ControlHandle;

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

// Forward Declaration
typedef struct StateSpace StateSpace;

void ControlSystem_InitHandle(ControlHandle *ctx, ControlArena *p,
                              ControlArena *s);
void ControlSystemDeInit();

control_vector_t PolyCoeffVector(ControlHandle *ctx, float *coeffs,
                                 size_t size);
int PolyCoeffVectorToStr(const control_vector_t *coeffs, char var, char *buffer,
                         size_t buffer_size);

control_vector_t AddCoeffVector(ControlHandle *ctx, const control_vector_t *a,
                                const control_vector_t *b);

TransferFunction TransferFunctionFromCoeffs(const control_vector_t *num,
                                            const control_vector_t *dem);
int TransferFunctionToStr(TransferFunction *tf, char var, char *buffer,
                          size_t buffer_size);

control_vector_t MultiplyPoly(ControlHandle *ctx, control_vector_t *a,
                              control_vector_t *b);

TransferFunction MultiplyTransferFunctions(ControlHandle *ctx,
                                           TransferFunction *G1,
                                           TransferFunction *G2);
TransferFunction UnityClosedLoop(ControlHandle *ctx, TransferFunction *G,
                                 float gain, TransferFunctionUnity unity);

StateSpace TransferFunctionToStateSpace(ControlHandle *ctx,
                                        TransferFunction *tf);
