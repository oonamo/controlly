#include "../internal_common.h"
#include "ss_internal.h"
#include <ccontrol/statespace.h>

void StateSpace_StepContinous(ControlHandle *ctx, StateSpace *ss, float dt)
{
    CCONTROL_UNUSED(ctx);

    if (ss->B.cols == 1 && ss->C.rows == 1)
    {
        __StateSpace_StepSISO(ss, dt);
    }
    else
    {
        __StateSpace_StepMIMO(ss, dt);
    }
}
