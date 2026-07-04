#ifndef _SS_INTERNAL
#define _SS_INTERNAL

#include <ccontrol/statespace.h>

void __StateSpace_StepSISO( StateSpace* ss, float dt);
void __StateSpace_StepMIMO( StateSpace* ss, float dt);

#endif
