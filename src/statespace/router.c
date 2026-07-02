#include "ss_internal.h"
#include <ccontrol/statespace.h>

void StateSpace_StepContinous(ControlHandle *ctx, StateSpace *ss, float dt)
{
    if (ss->B.cols == 1 && ss->C.rows == 1)
    {
        __StateSpace_StepSISO(ctx, ss, dt);
    }
    else
    {
        __StateSpace_StepMIMO(ctx, ss, dt);
    }
}
