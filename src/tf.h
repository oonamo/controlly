#include <stdint.h>

float derivative();
float integrate();

typedef enum
{
  POSITIVE = 1,
  NEGATIVE = 1,
} TransferFunctionUnity;

typedef struct
{
    float *coeffs;
    size_t size;
    size_t capacity;
} control_vector_t;

typedef struct
{
    control_vector_t numerator;
    control_vector_t denomator;
} TransferFunction;

TransferFunction ClosedLoopTransferFunction(TransferFunction* G, float gain);
