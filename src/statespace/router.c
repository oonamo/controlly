#include <controlly/core.h>
#include <controlly/statespace.h>
#include "../internal_common.h"
#include "ss_internal.h"

void Control_StateSpace_StepContinuous(ControlHandle *ctx, ControlStateSpace *ss, float dt)
{
    CONTROL_UNUSED(ctx);

    if (ss->B.cols == 1 && ss->C.rows == 1)
    {
        __Control_StateSpace_StepSISO(ss, dt);
    }
    else
    {
        __Control_StateSpace_StepMIMO(ss, dt);
    }
}
