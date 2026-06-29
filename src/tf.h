#include "arena.h"
#include "matrix.h"
#include <stdint.h>

float derivative();
float integrate();

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

void ControlSystemInit();

control_vector_t PolyCoeffVector(float *coeffs, size_t size);
int PolyCoeffVectorToStr(control_vector_t *coeffs, char var, char *buffer,
                         size_t buffer_size);

TransferFunction TransferFunctionFromCoeffs(control_vector_t num,
                                            control_vector_t dem);
int TransferFunctionToStr(TransferFunction* tf, char var, char* buffer, size_t buffer_size);

control_vector_t ConvolveCoeffVector(Arena *arena, control_vector_t *a,
                                     control_vector_t *b);

TransferFunction MultiplyTransferFunctions(TransferFunction *G1,
                                            TransferFunction *G2);
TransferFunction UnityClosedLoop(TransferFunction *G, float gain,
                                            TransferFunctionUnity unity);
