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

void ControlSystem_InitHandle(ControlHandle *hndl, ControlArena *p,
                              ControlArena *s);
void ControlSystemDeInit();

control_vector_t PolyCoeffVector(ControlHandle *hndl, float *coeffs,
                                 size_t size);
int PolyCoeffVectorToStr(control_vector_t *coeffs, char var, char *buffer,
                         size_t buffer_size);

TransferFunction TransferFunctionFromCoeffs(control_vector_t num,
                                            control_vector_t dem);
int TransferFunctionToStr(TransferFunction *tf, char var, char *buffer,
                          size_t buffer_size);

control_vector_t MultiplyPoly(ControlHandle *hndl, control_vector_t *a,
                              control_vector_t *b);

TransferFunction MultiplyTransferFunctions(ControlHandle *hndl,
                                           TransferFunction *G1,
                                           TransferFunction *G2);
TransferFunction UnityClosedLoop(ControlHandle *hndl, TransferFunction *G,
                                 float gain, TransferFunctionUnity unity);

StateSpace TransferFunctionToStateSpace(ControlHandle *hndl,
                                        TransferFunction *tf);
