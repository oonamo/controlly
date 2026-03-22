#include "arena.h"
#include <stdint.h>

float derivative();
float integrate();

typedef enum
{
    TF_UNITY_POSITIVE = 1,
    TF_UNITY_NEGATIVE = 1,
} TransferFunctionUnity;

typedef struct
{
    float *coeffs;
    size_t size;
    size_t capacity;
} control_vector_t;

typedef struct
{
    control_vector_t num;
    control_vector_t dem;
} TransferFunction;

void ControlSystemInit();

control_vector_t PolyCoeffVector(float *coeffs, size_t size);
int PolyCoeffVectorToStr(control_vector_t *coeffs, char var, char *buffer,
                          size_t buffer_size);

TransferFunction MultiplyTransferFunctrions(TransferFunction *G1,
                                            TransferFunction *G2);
TransferFunction ClosedLoopTransferFunction(TransferFunction *G, float gain,
                                            TransferFunctionUnity unity);
