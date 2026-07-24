/**
 * @file tf.h
 * @brief Transfer function and polynomial math module.
 *
 * This module provides dynamic polynomial arithmetic and transfer function
 * operations for linear time-invariant (LTI) systems.
 */
#ifndef _CONTROLLY_TF_H
#define _CONTROLLY_TF_H
#include <controlly/core.h>
#include <controlly/matrix.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Feedback loop type
 *
 * Defines the sign of the feedback summation junction
 */
typedef enum
{
    TF_FEEDBACK_NEGATIVE, /*!< Describes a system with a negative feedback loop (subtracts from
                             reference) */
    TF_FEEDBACK_POSITIVE, /*!< Describes a system with a positive feedback loop (adcds from
                             reference) */
} ControlFeedbackType;

/**
 * @brief Transfer function structure
 *
 * Represents a rational function $G(s) = \frac{N(s)}{D(s)}$
 */
typedef struct
{
    ControlVec num; /*!< Numerator polynomical of the transfer function */
    ControlVec den; /*!< Denominator polynomial of the transfer function */
} ControlTransferFunction;

/**
 * @brief A helper macro for initializing an empty ControlTransferFunction.
 */
#define CONTROL_EMPTY_TF ((ControlTransferFunction){0})

/**
 * @brief Allocates and initializes a polynomial in the scratch arena.
 *
 * @note Allocates from the `ctx->scratch` arena.
 *
 * @param[in,out] ctx    Pointer to the initialized control context.
 * @param[out]    out    Pointer to the destination polynomial vector.
 * @param[in]     coeffs Array of polynomial coefficients (highest order first).
 * @param[in]     size   Number of coefficients in the array.
 *
 * @return
 *        - CONTROL_OK on success.
 *        - CONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 *        - CONTROL_ERROR_OUT_OF_MEMORY if the scratch arena is exhausted.
 *        - CONTROL_ERROR_NULL_PTR if any pointer is NULL.
 */
ControlResult
Control_Poly_AllocScratch(ControlHandle *ctx, ControlVec *out, const float *coeffs, size_t size);

/**
 * @brief Allocates and initializes a polynomial in the persistent arena.
 *
 * @note Allocates from the `ctx->persistent` arena.
 *
 * @param[in,out] ctx    Pointer to the initialized control context.
 * @param[out]    out    Pointer to the destination polynomial vector.
 * @param[in]     coeffs Array of polynomial coefficients (highest order first).
 * @param[in]     size   Number of coefficients in the array.
 *
 * @return
 *        - CONTROL_OK on success.
 *        - CONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 *        - CONTROL_ERROR_OUT_OF_MEMORY if the persistent arena is exhausted.
 *        - CONTROL_ERROR_NULL_PTR if any pointer is NULL.
 */
ControlResult
Control_Poly_AllocPersistent(ControlHandle *ctx, ControlVec *out, const float *coeffs, size_t size);

/**
 * @brief Removes leading zeros from a polynomial to determine its true order.
 *
 * @note **Zero-Allocation Path:** This function executes in $O(1)$ time by safely
 *       advancing the internal data pointer. It does not allocate memory or copy data.
 * @note This function is alias-safe.
 *
 * @param[in]  ctx Pointer to the initialized control context.
 * @param[out] out Pointer to the destination polynomial.
 * @param[in]  v   Pointer to the input polynomial.
 *
 * @return
 *        - CONTROL_OK on success.
 *        - CONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 *        - CONTROL_ERROR_NULL_PTR if any pointer is NULL.
 */
ControlResult Control_Poly_Canonicalize(ControlHandle *ctx, ControlVec *out, const ControlVec *v);

/**
 * @brief Adds two polynomials together ($out = a + b$).
 *
 * Automatically aligns polynomials of different orders before addition.
 *
 * @note Performs allocations in the \p ctx->scratch arena.
 *       **Zero-Allocation Path:** If `out->capacity >= max(a->size, b->size)`,
 *       no memory is allocated.
 * @note This function is alias-safe.
 *
 * @param[in,out] ctx Pointer to the initialized control context.
 * @param[out]    out Pointer to the destination polynomial.
 * @param[in]     a   Pointer to the first polynomial.
 * @param[in]     b   Pointer to the second polynomial.
 *
 * @return
 *        - CONTROL_OK on success.
 *        - CONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 *        - CONTROL_ERROR_OUT_OF_MEMORY if the scratch arena is exhausted.
 *        - CONTROL_ERROR_NULL_PTR if any pointer is NULL.
 */
ControlResult
Control_Poly_Add(ControlHandle *ctx, ControlVec *out, const ControlVec *a, const ControlVec *b);

/**
 * @brief Multiplies (convolves) two polynomials ($out = a \times b$).
 *
 * The resulting polynomial will have a size of `a->size + b->size - 1`.
 *
 * @note Performs allocations in the \p ctx->scratch arena.
 *       **Zero-Allocation Path:** If `out->capacity >= a->size + b->size - 1`,
 *       no memory is allocated.
 * @note This function is alias-safe.
 *
 * @param[in,out] ctx Pointer to the initialized control context.
 * @param[out]    out Pointer to the destination polynomial.
 * @param[in]     a   Pointer to the first polynomial.
 * @param[in]     b   Pointer to the second polynomial.
 *
 * @return
 *        - CONTROL_OK on success.
 *        - CONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 *        - CONTROL_ERROR_OUT_OF_MEMORY if the scratch arena is exhausted.
 *        - CONTROL_ERROR_NULL_PTR if any pointer is NULL.
 */
ControlResult Control_Poly_Multiply(ControlHandle    *ctx,
                                    ControlVec       *out,
                                    const ControlVec *a,
                                    const ControlVec *b);

/**
 * @brief Constructs a Transfer Function from numerator and denominator polynomials.
 *
 * @note **Zero-Allocation Path:** This function directly maps the vectors and
 *       does not allocate new memory.
 *
 * @param[in]  ctx Pointer to the initialized control context.
 * @param[out] out Pointer to the destination transfer function.
 * @param[in]  num Pointer to the numerator polynomial.
 * @param[in]  dem Pointer to the denominator polynomial.
 *
 * @return
 *        - CONTROL_OK on success.
 *        - CONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 *        - CONTROL_ERROR_NULL_PTR if any pointer is NULL.
 */
ControlResult Control_TF_FromPoly(ControlHandle           *ctx,
                                  ControlTransferFunction *out,
                                  const ControlVec        *num,
                                  const ControlVec        *dem);

/**
 * @brief Multiplies two transfer functions in series ($out = G_1 \times G_2$).
 *
 * Automatically canonicalizes the resulting transfer function.
 *
 * @note Performs allocations in the \p ctx->scratch arena.
 *       **Zero-Allocation Path:** If the `out` transfer function has sufficient
 *       capacity in both its numerator and denominator vectors, no memory is allocated.
 * @note This function is alias-safe.
 *
 * @param[in,out] ctx Pointer to the initialized control context.
 * @param[out]    out Pointer to the destination transfer function.
 * @param[in]     G1  Pointer to the first transfer function.
 * @param[in]     G2  Pointer to the second transfer function.
 *
 * @return
 *        - CONTROL_OK on success.
 *        - CONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 *        - CONTROL_ERROR_OUT_OF_MEMORY if the scratch arena is exhausted.
 *        - CONTROL_ERROR_INVALID_ARGUMENT if G1 or G2 are improperly initialized.
 *        - CONTROL_ERROR_NULL_PTR if any pointer is NULL.
 */
ControlResult Control_TF_Multiply(ControlHandle                 *ctx,
                                  ControlTransferFunction       *out,
                                  const ControlTransferFunction *G1,
                                  const ControlTransferFunction *G2);

/**
 * @brief Computes the closed-loop transfer function of a system.
 *
 * Evaluates $H(s) = \frac{G(s)}{1 \pm k G(s)}$, where $G(s)$ is the plant/controller
 * combination, $k$ is the feedback gain, and the sign is determined by the feedback type.
 *
 * @note Performs allocations in the \p ctx->scratch arena.
 *       **Zero-Allocation Path:** If the `out` transfer function has sufficient
 *       capacity, no memory is allocated.
 * @note This function is alias-safe.
 *
 * @param[in,out] ctx   Pointer to the initialized control context.
 * @param[out]    out   Pointer to the destination closed-loop transfer function.
 * @param[in]     G     Pointer to the open-loop transfer function.
 * @param[in]     gain  The scalar gain ($k$) of the feedback path.
 * @param[in]     unity The sign of the feedback (TF_FEEDBACK_NEGATIVE or TF_FEEDBACK_POSITIVE).
 *
 * @return
 *        - CONTROL_OK on success.
 *        - CONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 *        - CONTROL_ERROR_OUT_OF_MEMORY if the scratch arena is exhausted.
 *        - CONTROL_ERROR_NULL_PTR if any pointer is NULL.
 */
ControlResult Control_TF_ClosedLoop(ControlHandle                 *ctx,
                                    ControlTransferFunction       *out,
                                    const ControlTransferFunction *G,
                                    float                          gain,
                                    ControlFeedbackType            unity);

/**
 * @brief Silently checks if a transfer function is valid and safe for math operations.
 *
 * Validates that pointers exist, vectors are sized greater than zero, and the
 * denominator's leading coefficient is not zero (to prevent divide-by-zero faults).
 *
 * @param[in] tf Pointer to the transfer function to check.
 * @return true if valid; false if improperly initialized.
 */
bool Control_TF_IsValid(const ControlTransferFunction *tf);

/**
 * @brief Validates a transfer function and throws a library error if invalid.
 *
 * Wraps `Control_TF_IsValid` but actively triggers the `ctx->on_error` callback
 * and returns the specific failure code.
 *
 * @param[in] ctx Pointer to the initialized control context.
 * @param[in] tf  Pointer to the transfer function to validate.
 *
 * @return
 *        - CONTROL_OK if valid.
 *        - CONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 *        - CONTROL_ERROR_INVALID_ARGUMENT if pointers are null, sizes are zero, or coefficents are
 * NULL.
 *        - CONTROL_ERROR_DIVIDE_BY_ZERO if the leading denominator coefficient is zero.
 */
ControlResult Control_TF_Validate(ControlHandle *ctx, const ControlTransferFunction *tf);

/**
 * @brief Copies a transfer function into persistent memory.
 *
 * Promotes temporary calculations into long-term storage. If the destination
 * transfer function `out` does not have sufficient capacity in its vectors,
 * it will automatically allocate memory from the persistent arena.
 *
 * @note Allocates from the \p ctx->persistent arena.
 *       **Zero-Allocation Path:** If both `out->num` and `out->den` possess
 *       sufficient capacity, no memory is allocated.
 * @note This function is alias-safe.
 *
 * @param[in,out] ctx Pointer to the initialized control context.
 * @param[out]    out Pointer to the destination transfer function.
 * @param[in]     tf  Pointer to the source transfer function to copy.
 *
 * @return
 *        - CONTROL_OK on success.
 *        - CONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 *        - CONTROL_ERROR_OUT_OF_MEMORY if the persistent arena is exhausted.
 *        - CONTROL_ERROR_NULL_PTR if any pointer is NULL.
 */
ControlResult Control_TF_Persist(ControlHandle                 *ctx,
                                 ControlTransferFunction       *out,
                                 const ControlTransferFunction *tf);

#endif // _CONTROLLY_TF_H
