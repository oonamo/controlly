#include <controlly/core.h>

const char *Control_GetErrorString(ControlResult result)
{
    switch (result)
    {
    case CONTROL_OK:
        return CONTROLLY_ERROR_OK_MSG;
    case CONTROL_ERROR_CTX_UNINITIALIZED:
        return CONTROLLY_ERROR_CTX_UNINITIALIZED_MSG;
    case CONTROL_ERROR_OUT_OF_MEMORY:
        return CONTROLLY_ERROR_OUT_OF_MEMORY_MSG;
    case CONTROL_ERROR_DIVIDE_BY_ZERO:
        return CONTROLLY_ERROR_DIVIDE_BY_ZERO_MSG;
    case CONTROL_ERROR_INVALID_ARGUMENT:
        return CONTROLLY_ERROR_INVALID_ARGUMENT_MSG;
    case CONTROL_ERROR_NULL_PTR:
        return CONTROLLY_ERROR_NULL_PTR_MSG;
    case CONTROL_ERROR_FEATURE_DISABLED:
        return CONTROLLY_ERROR_FEATURE_DISABLED_MSG;
    case CONTROL_ERROR_DIMENSION_MISMATCH:
        return CONTROLLY_ERROR_DIMENSION_MISMATCH_MSG;
    default:
        return CONTROLLY_ERROR_UNKNOWN_MSG;
    }
}

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
