#ifndef CCONTROL_ERROR_H
#define CCONTROL_ERROR_H

#include "arena.h"
#include <stddef.h>

typedef enum
{
    CCONTROL_OK = 0,
    CCONTROL_ERROR_OUT_OF_MEMORY,
    CCONTROL_ERROR_DIVIDE_BY_ZERO,
    CCONTROL_ERROR_INVALID_ARGUMENT,
    CCONTROL_ERROR_USE_DISABLE_FEATURE,
} ControlResult;

typedef void (*ControlErrorCallback)(ControlResult code, const char *message,
                                     void *user_data);


typedef struct
{
    ControlArena *persistent;
    ControlArena *scratch;

    ControlErrorCallback on_error;
    void *user_data;
} ControlHandle;

void Control_System_Init(ControlHandle *ctx, ControlArena *p,
                              ControlArena *s);
void Control_System_DeInit(ControlHandle *ctx);

#endif
