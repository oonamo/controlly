#include <controlly/core.h>

void Control_System_Init(ControlHandle *ctx, ControlArena *p, ControlArena *s)
{
    ctx->persistent = p;
    ctx->scratch    = s;
}

void Control_System_DeInit(ControlHandle *ctx)
{
    Control_Arena_Clear(ctx->scratch);
    Control_Arena_Clear(ctx->persistent);
}
