#ifndef _STATESPACE
#define _STATESPACE

#include "tf.h"
#include <ccontrol/core.h>
#include <ccontrol/matrix.h>

typedef matrix_t system_matrix_t;
typedef matrix_t input_matrix_t;
typedef matrix_t output_matrix_t;
typedef matrix_t feedback_matrix_t;

typedef struct
{
    system_matrix_t A;
    input_matrix_t B;
    output_matrix_t C;
    feedback_matrix_t D;

    vector_t y;
    vector_t u;
    vector_t x;
} StateSpace;

#define CCONTROL_EMPTY_STATESPACE (StateSpace){0}

void StateSpace_StepContinous(ControlHandle *ctx, StateSpace *ss, float dt);
StateSpace TransferFunctionToStateSpace(ControlHandle *ctx,
                                        TransferFunction *tf);

#endif
