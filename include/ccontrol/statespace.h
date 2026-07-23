#ifndef _STATESPACE
#define _STATESPACE

#include "tf.h"
#include <ccontrol/core.h>
#include <ccontrol/matrix.h>

typedef ControlMatrix ControlSystemMatrix;
typedef ControlMatrix ControlInputMatrix;
typedef ControlMatrix ControlOutputMatrix;
typedef ControlMatrix ControlFeedbackMatrix;

typedef struct
{
    ControlSystemMatrix A;
    ControlInputMatrix B;
    ControlOutputMatrix C;
    ControlFeedbackMatrix D;

    ControlVec y;
    ControlVec u;
    ControlVec x;
} ControlStateSpace;

#define CCONTROL_EMPTY_STATESPACE (ControlStateSpace){0}

void Control_StateSpace_StepContinuous(ControlHandle *ctx, ControlStateSpace *ss, float dt);
ControlResult Control_StateSpace_FromTF(ControlHandle *ctx, ControlStateSpace* out, const ControlTransferFunction *tf);

#endif
