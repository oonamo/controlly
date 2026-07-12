#include "internal_common.h"
#include <ccontrol/core.h>

void Control_System_Init(ControlHandle *ctx, ControlArena *p, ControlArena *s)
{
    ctx->persistent = p;
    ctx->scratch = s;
}

void Control_System_DeInit(ControlHandle *ctx) { CCONTROL_UNUSED(ctx); }
