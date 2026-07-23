#ifndef _TF
#define _TF
#include "core.h"
#include "matrix.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// TODO: Rename to ControlFeedbackType
/**
 * Feedback loop type
 */
typedef enum
{
    TF_FEEDBACK_NEGATIVE, /*!< Describes a system with a negative feedback loop */
    TF_FEEDBACK_POSITIVE, /*!< Describes a system with a positive feedback loop */
} ControlFeedbackUnity;

/**
 * Transfer function structure
 */
typedef struct
{
    ControlVec num; /*!< Numerator of the transfer function */
    ControlVec den; /*!< Denominator of the transfer function */
} ControlTransferFunction;

/**
 * An Empty ControlTransferFunction
 */
#define CCONTROL_EMPTY_TF ((ControlTransferFunction){0})

/*
 */
#define CONTROL_TF_INIT   {0}

ControlResult
Control_Poly_AllocScratch(ControlHandle *ctx, ControlVec *out, const float *coeffs, size_t size);

ControlResult
Control_Poly_AllocPersistent(ControlHandle *ctx, ControlVec *out, const float *coeffs, size_t size);

ControlResult Control_Poly_Canonicalize(ControlHandle *ctx, ControlVec *out, const ControlVec *v);

ControlResult
Control_Poly_Add(ControlHandle *ctx, ControlVec *out, const ControlVec *a, const ControlVec *b);

ControlResult Control_Poly_Multiply(ControlHandle *ctx,
                                    ControlVec *out,
                                    const ControlVec *a,
                                    const ControlVec *b);

ControlResult Control_TF_FromPoly(ControlHandle *ctx,
                                  ControlTransferFunction *out,
                                  const ControlVec *num,
                                  const ControlVec *dem);

ControlResult Control_TF_Multiply(ControlHandle *ctx,
                                  ControlTransferFunction *out,
                                  const ControlTransferFunction *G1,
                                  const ControlTransferFunction *G2);

ControlResult Control_TF_ClosedLoop(ControlHandle *ctx,
                                    ControlTransferFunction *out,
                                    const ControlTransferFunction *G,
                                    float gain,
                                    ControlFeedbackUnity unity);

bool Control_TF_IsValid(const ControlTransferFunction* tf);
ControlResult Control_TF_Validate(ControlHandle *ctx, const ControlTransferFunction *tf);

ControlResult Control_TF_Persist(ControlHandle *ctx,
                                 ControlTransferFunction *out,
                                 const ControlTransferFunction *tf);

#endif
