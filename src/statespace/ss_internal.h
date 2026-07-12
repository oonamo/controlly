#ifndef _SS_INTERNAL
#define _SS_INTERNAL

#include <ccontrol/statespace.h>

void __Control_StateSpace_StepSISO(ControlStateSpace *ss, float dt);
void __Control_StateSpace_StepMIMO(ControlStateSpace *ss, float dt);

#endif
