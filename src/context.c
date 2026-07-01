#include <ccontrol/context.h>

void ControlSystem_InitHandle(ControlHandle *hndl, ControlArena *p,
                              ControlArena *s)
{
    hndl->persistent = p;
    hndl->scratch = s;
}
