#ifndef _SS_INTERNAL_H
#define _SS_INTERNAL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <controlly/statespace.h>

void __Control_StateSpace_StepSISO(ControlStateSpace *ss, float dt);
void __Control_StateSpace_StepMIMO(ControlStateSpace *ss, float dt);

#ifdef __cplusplus
}
#endif

#endif // _SS_INTERNAL_H
