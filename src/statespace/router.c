#include "../internal_common.h"
#include "ss_internal.h"
#include <ccontrol/statespace.h>

void Control_StateSpace_StepContinuous(ControlHandle *ctx, ControlStateSpace *ss, float dt)
{
    CCONTROL_UNUSED(ctx);

    if (ss->B.cols == 1 && ss->C.rows == 1)
    {
        __Control_StateSpace_StepSISO(ss, dt);
    }
    else
    {
        __Control_StateSpace_StepMIMO(ss, dt);
    }
}
