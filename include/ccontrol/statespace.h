#ifndef _STATESPACE
#define _STATESPACE

#include "tf.h"
#include <ccontrol/core.h>
#include <ccontrol/matrix.h>

typedef ControlMatrix ControlSystemMatrixj;
typedef ControlMatrix ControlInputMatrix;
typedef ControlMatrix ControlOutputMatrix;
typedef ControlMatrix ControlFeedbackMatrix;

typedef struct
{
    ControlSystemMatrixj A;
    ControlInputMatrix B;
    ControlOutputMatrix C;
    ControlFeedbackMatrix D;

    ControlVec y;
    ControlVec u;
    ControlVec x;
} ControlStateSpace;

#define CCONTROL_EMPTY_STATESPACE (ControlStateSpace){0}

void Control_StateSpace_StepContinuous(ControlHandle *ctx, ControlStateSpace *ss, float dt);
ControlStateSpace Control_StateSpace_FromTF(ControlHandle *ctx, ControlTransferFunction *tf);

#endif
