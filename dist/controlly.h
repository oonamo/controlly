/* Controlly - Automated Header Library Conversion */

/*
 * ========================================
 * arena.h
 * ========================================
*/

/**
 * @file arena.h
 * @brief Lightweight memory implementation
 *
 * This module contains a fast O(1) bump allocator which
 * is designed to prevent memory fragmentation
 * The arena can be used standalone, without the need of any other header
 */
#ifndef _CONTROLLY_ARENA_H
#define _CONTROLLY_ARENA_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <stdint.h>

#ifndef CONTROLLY_ARENA_ALIGN_SIZE
    /*
     * @brief Size of arena alignment
     */
    #define CONTROLLY_ARENA_ALIGN_SIZE 8
#endif

/**
 * @brief In-place structure for the memory arena
 *
 * @warning Fields prefixed with an underscore (_) are considered private
 * and should not be modified by the user
 */
typedef struct
{
    uint8_t *_buffer;
    size_t   _capacity;
    size_t   _offset;
} ControlArena;

/**
 * @brief Initializes a new memory arena inside the provided backing buffer.
 *
 * This is an in-place allocator. The `ControlArena` struct is placed in the beginning
 * of the provided buffer. The actual usable capacity at init will be `capacity -
 * sizeof(ControlArena)`.
 *
 * @param[out] backing_buffer Pointer to a allocated chunk of memory.
 * @param[in]  capacity       Total size of the backing_buffer, in bytes.
 *
 * @return Pointer to an initialized ControlArena, or NULL if capacity is too small.
 */
ControlArena *Control_Arena_Create(void *backing_buffer, size_t capacity);

/**
 * @brief Resets arena's allocation offset to zero.
 *
 * This invalidates all pointers previously in the arena, while not
 * zeroing out the backing buffer, ensuring a O(1) operation.
 *
 * @warning Pointers allocated in the arena are not guaranteed to be NULL.
 * Using the pointers allocated in the arena is **undefined behavior**.
 *
 * @param[in,out] arena Pointer to the arena to clear.
 */
void Control_Arena_Clear(ControlArena *arena);

/**
 * @brief Calculates the remaining bytes in the arena.
 *
 * @warning On initialization, the result of `Control_Arena_RemainingSpace(arena)` will be less
 * then the original backing buffer capacity due to the internal header.
 *
 * @param[in] arena Pointer to the arena.
 *
 * @return Number of possible bytes available for allocation.
 */
size_t Control_Arena_RemainingSpace(ControlArena *arena);

/**
 * @brief Allocates a block of memory in the arena.
 *
 * @param[in,out] arena Pointer to the arena.
 * @param[in] size Number of bytes to allocate
 *
 * @return Pointer to newly allocated memory, or NULL if size is 0 or if the arena did not have
 * enough capacity.
 */
void *Control_Arena_Alloc(ControlArena *arena, size_t size);

#ifdef __cplusplus
}
#endif

#endif // _CONTROLLY_ARENA_H

/*
 * ========================================
 * core.h
 * ========================================
*/

/**
 * @file core.h
 * @brief Core module of Controlly
 *
 * This module contains the core definitions needed for the Controlly library.
 * It exposes error codes, memory management, and the ControlHandle for handling library operations
 */
#ifndef _CONTROLLY_CORE_H
#define _CONTROLLY_CORE_H

#ifdef __cplusplus
extern "C"
{
#endif


#include <stddef.h>

#ifndef CONTROLLY_PRIVATE_API
    #define CONTROLLY_PRIVATE_API static
#endif

#ifndef CONTROLLY_INLINE_API
    #define CONTROLLY_INLINE_API inline
#endif

/**
 * @brief Results of Control system operations
 */
typedef enum
{
    CONTROL_OK = 0,                  /*!< Operation success */
    CONTROL_ERROR_CTX_UNINITIALIZED, /*!< A function that requires context was unintialized */
    CONTROL_ERROR_OUT_OF_MEMORY,     /*!< Out of memory (arena is full) */
    CONTROL_ERROR_DIVIDE_BY_ZERO,    /*!< A division by zero was attempted */
    CONTROL_ERROR_INVALID_ARGUMENT,  /*!< An invalid parameter was passed to a function */
    CONTROL_ERROR_NULL_PTR,          /*!< A required pointer parameter was NULL */
    CONTROL_ERROR_FEATURE_DISABLED,  /*!< A disabled feature was used */

    /* Math Errors */
    CONTROL_ERROR_DIMENSION_MISMATCH, /*!< Mismatched dimensions error */
} ControlResult;

/**
 * @defgroup ErrorMessages Customizable Error Strings
 * @brief Default string literals passed to the error callback
 *
 * These macros define the default strings used when a library error occurs.
 * Override them at compile time for verbosity, storage, or performance reasons.
 * @{
 */

/** @brief String for CONTROL_OK  */
#ifndef CONTROLLY_ERROR_OK_MSG
    #define CONTROLLY_ERROR_OK_MSG "Operation success"
#endif

/** @brief String for CONTROL_ERROR_CTX_UNINITIALIZED  */
#ifndef CONTROLLY_ERROR_CTX_UNINITIALIZED_MSG
    #define CONTROLLY_ERROR_CTX_UNINITIALIZED_MSG "Context was unintialized"
#endif

/** @brief String for CONTROL_ERROR_OUT_OF_MEMORY  */
#ifndef CONTROLLY_ERROR_OUT_OF_MEMORY_MSG
    #define CONTROLLY_ERROR_OUT_OF_MEMORY_MSG "Out of memory"
#endif

/** @brief String for CONTROL_ERROR_DIVIDE_BY_ZERO  */
#ifndef CONTROLLY_ERROR_DIVIDE_BY_ZERO_MSG
    #define CONTROLLY_ERROR_DIVIDE_BY_ZERO_MSG "Division by zero"
#endif

/** @brief String for CONTROL_ERROR_INVALID_ARGUMENT  */
#ifndef CONTROLLY_ERROR_INVALID_ARGUMENT_MSG
    #define CONTROLLY_ERROR_INVALID_ARGUMENT_MSG "An invalid argument was passed"
#endif

/** @brief String for CONTROL_ERROR_NULL_PTR  */
#ifndef CONTROLLY_ERROR_NULL_PTR_MSG
    #define CONTROLLY_ERROR_NULL_PTR_MSG "A null pointer was passed"
#endif

/** @brief String for CONTROL_ERROR_FEATURE_DISABLED  */
#ifndef CONTROLLY_ERROR_FEATURE_DISABLED_MSG
    #define CONTROLLY_ERROR_FEATURE_DISABLED_MSG "Disabled feature was used"
#endif

/** @brief String for CONTROL_ERROR_DIMENSION_MISMATCH  */
#ifndef CONTROLLY_ERROR_DIMENSION_MISMATCH_MSG
    #define CONTROLLY_ERROR_DIMENSION_MISMATCH_MSG "Dimensions are mismatched"
#endif

/** @brief String for CONTROL_ERROR_UNKNOWN  */
#ifndef CONTROLLY_ERROR_UNKNOWN_MSG
    #define CONTROLLY_ERROR_UNKNOWN_MSG "An unknown error occured"
#endif

/** @} */ // End of ErrorMessages group

/**
 * @def CONTROLLY_VERBOSE_ERROR_FMT
 * @brief Contextual data to forward to Context's on_error function via the verbose_data parameter
 *
 * If @c CONTROLLY_VERBOSE_ERRORS is enabled, this defaults to the standard C macro @c __func__
 * If @c CONTROLLY_VERBOSE_ERRORS is disabled, this is set to NULL
 */
#ifdef CONTROLLY_VERBOSE_ERRORS
    #ifndef CONTROLLY_VERBOSE_ERROR_FMT
        #define CONTROLLY_VERBOSE_ERROR_FMT __func__
    #endif
#else
    #define CONTROLLY_VERBOSE_ERROR_FMT NULL
#endif

/**
 * @brief Gets an error string from a ControlResult
 *
 * @param[in] result The result error code
 * @return string of the resulting error code
 */
const char *Control_GetErrorString(ControlResult result);

/**
 * @brief Function signature for handling errors.
 *
 * @param[in] code         The resulting error code thrown by the library.
 * @param[in] message      The resulting error code thrown by the library.
 * @param[in] verbose_data Contextual information (e.g. the function name). WARNING: This may be
 *                         NULL if not compiled with verbose errors
 * @param[in] user_data    A pointer to user-defined data passed from the ControlHandle struct.
 */
typedef void (*ControlErrorCallback)(ControlResult code,
                                     const char   *message,
                                     const char   *verbose_data,
                                     void         *user_data);

/**
 * @brief Structure for Controlly context
 */
typedef struct
{
    ControlArena *persistent; /*!< Arena that holds permenant data */
    ControlArena *scratch;    /*!< Arena that holds temporary data */

    ControlErrorCallback
          on_error;  /*!< Calls the on_error function with context if an error occurs */
    void *user_data; /*!< Unique data for tracking contexts */
} ControlHandle;

/**
 * @brief Initializes the control system context.
 *
 * @param[in,out] ctx Pointer to the ControlHandle to initialize.
 * @param[in] p   Pointer to Persistent ControlArena.
 * @param[in] s   Pointer to Scratch ControlArena.
 */
void Control_System_Init(ControlHandle *ctx, ControlArena *p, ControlArena *s);

/**
 * @brief De-initializes the control system and clears the associated memory.
 *
 * @param[in,out] ctx Pointer to the initialized ControlHandle.
 */
void Control_System_DeInit(ControlHandle *ctx);

#ifdef __cplusplus
}
#endif
#endif // _CONTROLLY_CORE_H

/*
 * ========================================
 * matrix.h
 * ========================================
*/

/**
 * @file matrix.h
 * @brief Minimal vector and matrix math implementations
 *
 * This module contains a minimal support of common linear algebra functions for use with the
 * Controlly library.
 */
#ifndef _CONTROLLY_MATRIX_H
#define _CONTROLLY_MATRIX_H

#ifdef __cplusplus
extern "C"
{
#endif


#include <stdbool.h>
#include <stddef.h>

/**
 * @brief A standard definition of a Vector.
 */
typedef struct
{
    size_t size;     /*!< Number of active elements in the ControlVec. */
    size_t capacity; /*!< Allocated capacity in the ControlVec */
    float *coeffs;   /*!< Array of coefficients in the ControlVec */
} ControlVec;

/**
 * @brief A helper macro for initializing an empty ControlVec.
 */
#define CONTROL_EMPTY_VEC ((ControlVec){0})

/**
 * @brief A standard definition of a matrix.
 */
typedef struct
{
    size_t rows; /*!< Number of rows in the ControlMatrix */
    size_t cols; /*!< Number of columns in the ContrlMatrix */
    float *data; /*!< Row-major array of coefficients in the ControlMatrix */
} ControlMatrix;

/**
 * @brief A helper macro for initializing an empty ControlMatrix.
 */
#define CONTROL_EMPTY_MATRIX ((ControlMatrix){0})

/**
 * @brief Allocates a ControlVec in a scratch arena.
 *
 * @note Allocates from the `ctx->scratch` arena.
 *
 * @param[in,out] ctx  Pointer to the initialized control context.
 * @param[out]    out  Pointer to the the destination ControlVec
 * @param[in]     size Amount of elements to allocate
 *
 * @return CONTROL_OK on success.
 * @return CONTROL_ERROR_OUT_OF_MEMORY if the scratch arena is exhausted.
 * @return CONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 * @return CONTROL_ERROR_NULL_PTR if the out vector is NULL.
 */
ControlResult Control_Vec_AllocScratch(ControlHandle *ctx, ControlVec *out, size_t size);

/**
 * @brief Allocates a ControlVec in a persistent arena.
 *
 * @note Allocates from the `ctx->persistent` arena.
 *
 * @param[in,out]  ctx  Pointer to the initialized control context.
 * @param[out] out  Pointer to the destination ControlVec
 * @param[in]  size Amount of elements to allocate
 *
 * @return CONTROL_OK on success.
 * @return CONTROL_ERROR_OUT_OF_MEMORY if the scratch persistent is exhausted.
 * @return CONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 * @return CONTROL_ERROR_NULL_PTR if the out vector is NULL.
 */
ControlResult Control_Vec_AllocPersistent(ControlHandle *ctx, ControlVec *out, size_t size);

/**
 * @brief Copies a vector into persistent memory.
 *
 * This function should be used to promote temporary calculations into persistent memory.
 * If the destination vector `out` does not have sufficient capacity to hold `v`, out will allocate
 * memory from the persistent arena.
 *
 * @note This function is alias-safe
 *
 * @param[in,out] ctx Pointer to the initialized control context.
 * @param[out]    out Pointer to the destination ControlVec>
 * @param[in]     v   Pointer to the source ControlVec to copy.
 *
 * @return CONTROL_OK on success.
 * @return CONTROL_ERROR_OUT_OF_MEMORY if the scratch persistent is exhausted.
 * @return CONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 * @return CONTROL_ERROR_NULL_PTR if the out vector is NULL.
 */
ControlResult Control_Vec_Persist(ControlHandle *ctx, ControlVec *out, const ControlVec *v);

/**
 * @brief Allocates a ControlMatrix in the persistent arena.
 *
 * @note Allocates from the `ctx->persistent` arena.
 *
 * @param[in]  ctx  Pointer to the initialized control context.
 * @param[out] out  Pointer to the destination ControlMatrix.
 * @param[in]  rows Number of rows to allocate.
 * @param[in]  cols Number of columns to allocate.
 *
 * @return CONTROL_OK on success.
 * @return CONTROL_ERROR_OUT_OF_MEMORY if the scratch persistent is exhausted.
 * @return CONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 * @return CONTROL_ERROR_NULL_PTR if the out vector is NULL.
 */
ControlResult
Control_Matrix_AllocPersistent(ControlHandle *ctx, ControlMatrix *out, size_t rows, size_t cols);

/**
 * @brief Allocates a ControlMatrix in the scratch arena.
 *
 * @note Allocates from the `ctx->scratch` arena.
 *
 * @param[in]  ctx  Pointer to the initialized control context.
 * @param[out] out  Pointer to the destination ControlMatrix.
 * @param[in]  rows Number of rows to allocate.
 * @param[in]  cols Number of columns to allocate.
 *
 * @return CONTROL_OK on success.
 * @return CONTROL_ERROR_OUT_OF_MEMORY if the scratch scratch is exhausted.
 * @return CONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 * @return CONTROL_ERROR_NULL_PTR if the out vector is NULL.
 */
ControlResult
Control_Matrix_AllocScratch(ControlHandle *ctx, ControlMatrix *out, size_t rows, size_t cols);

/**
 * @brief Allocates a ControlMatrix in the persistent arena.
 *
 * @note **Zero-Allocation**: If `out->rows == m->rows && out->cols == m->cols`, no memory is
 * allocated
 *
 * @param[in,out] ctx  Pointer to the initialized control context.
 * @param[out]    out  Pointer to the destination ControlMatrix.
 * @param[in]     m    Pointer to the source ControlMatrix to copy.
 *
 * @return CONTROL_OK on success.
 * @return CONTROL_ERROR_OUT_OF_MEMORY if the scratch persistent is exhausted.
 * @return CONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 * @return CONTROL_ERROR_NULL_PTR if the out vector is NULL.
 */
ControlResult
Control_Matrix_Persist(ControlHandle *ctx, ControlMatrix *out, const ControlMatrix *m);

/**
 * @brief Multiplies a matrix by a vector (out = M * v).
 *
 * @note Allocates from the `ctx->scratch` arena.
 *
 * Calculates the dot product of the matrix rows and the vector.
 * The number of columns in the matrix must exactly equal the size of the vector.
 * (m x n * n x 1 = n x 1)
 *
 * @note This function performs temporary allocations in the scratch arena to ensure
 *       it is alias-safe (e.g., `out` and `v` can point to the same vector).
 *
 * @param[in,out] ctx Pointer to the initialized control context.
 * @param[out]    out Pointer to the destination ControlVec.
 * @param[in]     m   Pointer to the input ControlMatrix.
 * @param[in]     v   Pointer to the input ControlVec.
 *
 * @return CONTROL_OK on success.
 * @return CONTROL_ERROR_DIMENSION_MISMATCH if m->cols != v->size.
 * @return CONTROL_ERROR_NULL_PTR if any pointer is NULL.
 */
ControlResult Control_Matrix_MultiplyVec(ControlHandle       *ctx,
                                         ControlVec          *out,
                                         const ControlMatrix *m,
                                         const ControlVec    *v);

/**
 * @brief Adds two vectors element-wise (out = lhs + rhs).
 *
 * This function sums the two input vectors of the same size.
 * If the destination vector `out` does not have sufficient space to store the result,
 * it will be allocated automatically.
 *
 * @note Performs allocation in the `ctx->scratch` arena.
 *       **Zero-Allocation**: If `out->capacity >= lhs->size`, no memory is allocated
 *
 * @note This function operates in-place and is alias-safe.
 *
 * @param[in,out] ctx Pointer to the initialized control context.
 * @param[out]    out Pointer to the destination ControlVec.
 * @param[in]     lhs Pointer to the left-hand side ControlVec.
 * @param[in]     rhs Pointer to the right-hand side ControlVec.
 *
 * @return CONTROL_OK on success.
 * @return CONTROL_ERROR_DIMENSION_MISMATCH if lhs->size != rhs->size.
 * @return CONTROL_ERROR_NULL_PTR if any pointer is NULL.
 */
ControlResult
Control_Vec_Add(ControlHandle *ctx, ControlVec *out, const ControlVec *lhs, const ControlVec *rhs);

/**
 * @brief Scales a vector by a scalar multiplier (out = v * scalar).
 *
 * Multiplies each element in the vector by a scalar.
 * If the destination vector `out` does not have sufficient space to store the result,
 * it will be allocated automatically.
 *
 * @note Performs allocation in the `ctx->scratch` arena.
 *       **Zero-Allocation**: If `out->capacity >= v->size`, no memory is allocated
 *
 * @note This function operates in-place and is alias-safe.
 *
 * @param[in,out] ctx    Pointer to the initialized control context.
 * @param[out]    out    Pointer to the destination ControlVec.
 * @param[in]     v      Pointer to the input ControlVec.
 * @param[in]     scalar The float multiplier.
 *
 * @return CONTROL_OK on success.
 * @return CONTROL_ERROR_NULL_PTR if any pointer is NULL.
 */
ControlResult
Control_Vec_Scale(ControlHandle *ctx, ControlVec *out, const ControlVec *v, float scalar);

/**
 * @brief Checks if a ControlMatrix is valid and initialized.
 *
 * @param[in] m Pointer to the ControlMatrix to check.
 * @return true if the matrix pointer and its data array are non-null; false otherwise.
 */
bool Control_Matrix_IsValid(const ControlMatrix *m);

/**
 * @brief Checks if a ControlVec is valid and initialized.
 *
 * @param[in] v Pointer to the ControlVec to check.
 * @return true if the vector pointer and its coefficient array are non-null; false otherwise.
 */
bool Control_Vec_IsValid(const ControlVec *v);

#ifdef __cplusplus
}
#endif
#endif // _CONTROLLY_MATRIX_H

/*
 * ========================================
 * tf.h
 * ========================================
*/

/**
 * @file tf.h
 * @brief Transfer function and polynomial math module.
 *
 * This module provides dynamic polynomial arithmetic and transfer function
 * operations for linear time-invariant (LTI) systems.
 */
#ifndef _CONTROLLY_TF_H
#define _CONTROLLY_TF_H

#ifdef __cplusplus
extern "C"
{
#endif



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
 *        - CONTROL_ERROR_INVALID_ARGUMENT if pointers are null, sizes are zero, or coefficents
 * are NULL.
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

#ifdef __cplusplus
}
#endif
#endif // _CONTROLLY_TF_H

/*
 * ========================================
 * statespace.h
 * ========================================
*/

/**
 * @file statespace.h
 * @brief State space module of Controlly
 *
 * This module contains support for modeling of state space systems
 */
#ifndef _CONTROLLY_STATESPACE_H
#define _CONTROLLY_STATESPACE_H


#ifdef __cplusplus
extern "C"
{
#endif







typedef ControlMatrix ControlSystemMatrix;
typedef ControlMatrix ControlInputMatrix;
typedef ControlMatrix ControlOutputMatrix;
typedef ControlMatrix ControlFeedbackMatrix;

/**
 * @brief A structure holding information on the State Space systems equation.
 */
typedef struct
{
    ControlSystemMatrix   A; /*!< The system matrix */
    ControlInputMatrix    B; /*!< The input matrix */
    ControlOutputMatrix   C; /*!< The output matrix */
    ControlFeedbackMatrix D; /*!< The feedback matrix */

    ControlVec y; /*!< The output vector */
    ControlVec u; /*!< The input vector  */
    ControlVec x; /*!< The state vector */
} ControlStateSpace;

/**
 * @brief Helper function for initializing an empty State Space structure.
 */
#define CONTROL_EMPTY_STATESPACE (ControlStateSpace){0}

/**
 * @brief Mathematically steps into the next iteration of the state space equation.
 *
 * @note Both SISO and MIMO systems are supported.
 *
 * @note Uses Euler integration.
 *
 * @warning Large `dt` will result in non-continuous like behavior. It is recommended to clamp `dt`
 * to an appropriate value.
 *
 * @param[in,out] ctx Pointer to the initialized control context.
 * @param[in,out] ss  Pointer to the ControlStateSpace model.
 * @param[in]     dt  Continuous time step to use.
 */
void Control_StateSpace_StepContinuous(ControlHandle *ctx, ControlStateSpace *ss, float dt);

/**
 * @brief Transforms a ControlTransferFunction into a ControlStateSpace
 *
 * This function uses the **Controllable Canonical Form** model to convert the transfer function
 * into a state space system.
 *
 * @note Performs allocation in the `ctx->persistent` arena.
 *
 * @param[in,out] ctx Pointer to the initialized control context.
 * @param[out]    out Pointer to the destination ControlStateSpace.
 * @param[in]     tf  Pointer to the transform function to convert.
 *
 * @return CONTROL_OK on success.
 * @return CONTROL_ERROR_OUT_OF_MEMORY if the persistent arena is exhausted.
 * @return CONTROL_ERROR_INVALID_ARGUMENT if the transfer function is invalid or improper.
 * @return CONTROL_ERROR_NULL_PTR if any pointer is NULL.
 */
ControlResult Control_StateSpace_FromTF(ControlHandle                 *ctx,
                                        ControlStateSpace             *out,
                                        const ControlTransferFunction *tf);

#ifdef __cplusplus
}
#endif
#endif // _CONTROLLY_STATESPACE_H

/*
 * ========================================
 * pid.h
 * ========================================
*/

/**
 * @file pid.h
 * @brief Proportional-Integral-Derivative (PID) controller module.
 *
 * This module provides a standard PID controller implementation with
 * optional anti-windup and output clamping features.
 */
#ifndef _CONTROLLY_PID_H
#define _CONTROLLY_PID_H

#ifdef __cplusplus
extern "C"
{
#endif



#include <stdbool.h>

/**
 * @brief Configuration structure for PID controller
 */
typedef struct
{
    bool  enable_anti_windup; /*!< If enabled, clamps integral accumulator to prevent windup */
    float max_out;            /*!< Maximum allowable output (if `enable_anti_windup` is enabled) */
    float min_out;            /*!< Minimum allowable output (if `enable_anti_windup` is enabled) */
} ControlPIDConfig;

/**
 * @brief PID Controller state and configuration structure
 */
typedef struct
{
    float kp; /*!< Proportional gain */
    float kd; /*!< Derivative gain */
    float ki; /*!< Integral gain */

    float integral;   /*!< Internal integral accumulator for the integral term */
    float prev_error; /*!< Internal memory of previous error for derivative gain */

    ControlPIDConfig config; /*!< PID configuration structure */
} ControlPIDController;

/**
 * @brief Initializes a PID controller with the specified gains and configuration.
 *
 * @param[out] pid    Pointer to the PID controller to initialize.
 * @param[in]  kp     Proportional gain.
 * @param[in]  ki     Integral gain.
 * @param[in]  kd     Derivative gain.
 * @param[in]  config Pointer to the configuration struct. If NULL, anti-windup and limits are
 * disabled.
 */
void Control_PID_Init(
    ControlPIDController *pid, float kp, float ki, float kd, const ControlPIDConfig *config);

/**
 * @brief Resets the internal state of the PID controller.
 *
 * Clears the accumulated integral and previous error. This should be called
 * whenever the controller is re-enabled or the setpoint changes drastically.
 *
 * @param[in,out] pid Pointer to the initialized PID controller.
 */
void Control_PID_Reset(ControlPIDController *pid);

/**
 * @brief Computes the next control effort for the PID controller.
 *
 * Calculates the error (`target - measurement`) and updates the internal
 * integral and derivative states using Euler integration and differentiation.
 *
 * @param[in,out] pid         Pointer to the initialized PID controller.
 * @param[in]     target      The desired setpoint.
 * @param[in]     measurement The current measured state of the system.
 * @param[in]     dt          Continuous time step (delta time) since the last update.
 *
 * @return The computed control effort, clamped to the configured limits (if applicable).
 */
float Control_PID_Update(ControlPIDController *pid, float target, float measurement, float dt);

/**
 * @brief Converts a PID controller's gains into a continuous-time Transfer Function.
 *
 * Maps the PID gains to standard polynomial form: $C(s) = \frac{K_d s^2 + K_p s + K_i}{s}$.
 *
 * @note Allocates from the \p ctx->persistent arena.
 *       **Zero-Allocation Path:** If the `out` transfer function already possesses
 *       sufficient capacity in both its numerator and denominator vectors, no memory is
 * allocated.
 *
 * @param[in,out] ctx Pointer to the initialized control context.
 * @param[out]    out Pointer to the destination transfer function.
 * @param[in]     pid Pointer to the source PID controller.
 *
 * @return CONTROL_OK on success.
 * @return CONTROL_ERROR_OUT_OF_MEMORY if the persistent arena is exhausted.
 * @return CONTROL_ERROR_NULL_PTR if any pointer is NULL.
 */
ControlResult
Control_PID_ToTF(ControlHandle *ctx, ControlTransferFunction *out, const ControlPIDController *pid);

#ifdef __cplusplus
}
#endif
#endif // _CONTROLLY_PID_H


#ifdef CONTROLLY_IMPLEMENTATION

/*
 * ----------------------------------------
 * internal_common.h
 * ----------------------------------------
*/

#ifndef _INTERNAL_COMMON
#define _INTERNAL_COMMON

#define CONTROL_UNUSED(x) (void)(x)

#define CONTROL_ALIGN_UP(size, align) (((size_t)(size) + ((size_t)(align) - 1)) & ~((size_t)(align) - 1))

#define CONTROL_THROW(ctx, code)                                                                   \
    do                                                                                             \
    {                                                                                              \
        if ((ctx) && (ctx)->on_error)                                                              \
        {                                                                                          \
            (ctx)->on_error((code),                                                                \
                            Control_GetErrorString(code),                                          \
                            CONTROLLY_VERBOSE_ERROR_FMT,                                           \
                            (ctx)->user_data);                                                     \
        }                                                                                          \
    } while (0)

#define CONTROL_REQUIRE(ctx, condition, err_code)                                                  \
    do                                                                                             \
    {                                                                                              \
        if (!(condition))                                                                          \
        {                                                                                          \
            CONTROL_THROW(ctx, err_code);                                                          \
            return (err_code);                                                                     \
        }                                                                                          \
    } while (0)

#define CONTROL_CHECK_CTX(ctx) CONTROL_REQUIRE((ctx), (ctx), CONTROL_ERROR_CTX_UNINITIALIZED);

#define CONTROL_CHECK_OUT(ctx, out) CONTROL_REQUIRE((ctx), (out), CONTROL_ERROR_NULL_PTR)

#define CONTROL_CHECK_NOT_NULL(ctx, ptr) CONTROL_REQUIRE((ctx), (ptr), CONTROL_ERROR_NULL_PTR)

#define CONTROL_TRY(expr)                                                                          \
    do                                                                                             \
    {                                                                                              \
        ControlResult _controlly_status_try = (expr);                                              \
        if (_controlly_status_try != CONTROL_OK)                                                   \
        {                                                                                          \
            return _controlly_status_try;                                                          \
        }                                                                                          \
                                                                                                   \
    } while (0)

#endif

/*
 * ----------------------------------------
 * ss_internal.h
 * ----------------------------------------
*/

#ifndef _SS_INTERNAL_H
#define _SS_INTERNAL_H

#ifdef __cplusplus
extern "C"
{
#endif



void __Control_StateSpace_StepSISO(ControlStateSpace *ss, float dt);
void __Control_StateSpace_StepMIMO(ControlStateSpace *ss, float dt);

#ifdef __cplusplus
}
#endif

#endif // _SS_INTERNAL_H

/*
 * ----------------------------------------
 * core.c
 * ----------------------------------------
*/




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

/*
 * ----------------------------------------
 * arena.c
 * ----------------------------------------
*/



#include <stddef.h>
#include <stdint.h>

ControlArena *Control_Arena_Create(void *backing_buffer, size_t capacity)
{
    if (capacity <= sizeof(ControlArena))
    {
        return NULL;
    }
    ControlArena *arena = (ControlArena *)backing_buffer;
    arena->_buffer      = (uint8_t *)backing_buffer + sizeof(ControlArena);
    arena->_capacity    = capacity - sizeof(ControlArena);
    arena->_offset      = 0;

    return arena;
}

void Control_Arena_Clear(ControlArena *arena)
{
    arena->_offset = 0;
}

size_t Control_Arena_RemainingSpace(ControlArena *arena)
{
    if (!arena)
    {
        return 0;
    }

    return arena->_capacity - arena->_offset;
}

void *Control_Arena_Alloc(ControlArena *a, size_t size)
{
    if (size == 0)
    {
        return NULL;
    }

    size_t align_size = CONTROL_ALIGN_UP(size, CONTROLLY_ARENA_ALIGN_SIZE);
    if (a->_offset + align_size <= a->_capacity)
    {
        void *ptr = &a->_buffer[a->_offset];
        a->_offset += align_size;
        return ptr;
    }

    return NULL;
}

/*
 * ----------------------------------------
 * matrix.c
 * ----------------------------------------
*/




#include <stdbool.h>
#include <stddef.h>


// TODO: Add testing
//  1. Does it currently support aliasing?

static ControlResult
__Control_Vec_AllocInArena(ControlHandle *ctx, ControlVec *out, ControlArena *a, size_t size)
{

    out->size   = 0;
    out->coeffs = (float *)Control_Arena_Alloc(a, size * sizeof(float));
    CONTROL_REQUIRE(ctx, out->coeffs, CONTROL_ERROR_OUT_OF_MEMORY);

    out->capacity = size;

    // TODO: Does this need to be reset?
    for (size_t i = 0; i < size; i++)
    {
        out->coeffs[i] = 0.0f;
    }
    return CONTROL_OK;
}

ControlResult Control_Vec_AllocScratch(ControlHandle *ctx, ControlVec *out, size_t size)
{
    CONTROL_CHECK_CTX(ctx);
    CONTROL_CHECK_OUT(ctx, out);

    CONTROL_TRY(__Control_Vec_AllocInArena(ctx, out, ctx->scratch, size));
    return CONTROL_OK;
}
ControlResult Control_Vec_AllocPersistent(ControlHandle *ctx, ControlVec *out, size_t size)

{
    CONTROL_CHECK_CTX(ctx);
    CONTROL_CHECK_OUT(ctx, out);

    CONTROL_TRY(__Control_Vec_AllocInArena(ctx, out, ctx->persistent, size));
    return CONTROL_OK;
}

ControlResult Control_Vec_Persist(ControlHandle *ctx, ControlVec *out, const ControlVec *v)
{
    CONTROL_CHECK_CTX(ctx);
    CONTROL_CHECK_OUT(ctx, out);

    if (out->capacity < v->size)
    {
        CONTROL_TRY(Control_Vec_AllocPersistent(ctx, out, v->size));
    }

    out->size = v->size;

    // TODO: Abstract into some sort of memcpy (either from ControlHandle or macro)
    for (size_t i = 0; i < v->size; i++)
    {
        out->coeffs[i] = v->coeffs[i];
    }

    return CONTROL_OK;
}

ControlResult __Control_Matrix_Alloc(
    ControlHandle *ctx, ControlMatrix *out, ControlArena *a, size_t rows, size_t cols)
{
    out->data = (float *)Control_Arena_Alloc(a, rows * cols * sizeof(float));
    CONTROL_REQUIRE(ctx, out->data, CONTROL_ERROR_OUT_OF_MEMORY);

    // TODO: Does this need to be set to 0.0f?
    for (size_t i = 0; i < rows; i++)
    {
        for (size_t j = 0; j < cols; j++)
        {
            out->data[i * cols + j] = 0.0f;
        }
    }

    out->rows = rows;
    out->cols = cols;
    return CONTROL_OK;
}

ControlResult
Control_Matrix_AllocScratch(ControlHandle *ctx, ControlMatrix *out, size_t rows, size_t cols)
{
    CONTROL_CHECK_CTX(ctx);
    CONTROL_CHECK_OUT(ctx, out);

    CONTROL_TRY(__Control_Matrix_Alloc(ctx, out, ctx->scratch, rows, cols));
    return CONTROL_OK;
}

ControlResult
Control_Matrix_AllocPersistent(ControlHandle *ctx, ControlMatrix *out, size_t rows, size_t cols)
{
    CONTROL_CHECK_CTX(ctx);
    CONTROL_CHECK_OUT(ctx, out);

    CONTROL_TRY(__Control_Matrix_Alloc(ctx, out, ctx->persistent, rows, cols));
    return CONTROL_OK;
}

ControlResult Control_Matrix_Persist(ControlHandle *ctx, ControlMatrix *out, const ControlMatrix *m)
{
    CONTROL_CHECK_CTX(ctx);
    CONTROL_CHECK_OUT(ctx, out);

    if (out->rows != m->rows || out->cols != m->cols)
    {
        CONTROL_TRY(Control_Matrix_AllocPersistent(ctx, out, m->rows, m->cols));
    }

    // TODO: Abstract into CONTROL_MEMCPY
    for (size_t i = 0; i < m->rows; i++)
    {
        for (size_t j = 0; j < m->cols; j++)
        {
            out->data[i * m->cols + j] = m->data[i * m->cols + j];
        }
    }

    return CONTROL_OK;
}

ControlResult __Control_Vec_CreateInArenaRaw(ControlHandle *ctx,
                                             ControlVec    *out,
                                             ControlArena  *a,
                                             size_t         capacity)
{
    out->size   = 0;
    out->coeffs = (float *)Control_Arena_Alloc(a, capacity * sizeof(float));
    CONTROL_REQUIRE(ctx, out->coeffs, CONTROL_ERROR_OUT_OF_MEMORY);

    out->capacity = capacity;
    return CONTROL_OK;
}

ControlResult Control_Matrix_MultiplyVec(ControlHandle       *ctx,
                                         ControlVec          *out,
                                         const ControlMatrix *m,
                                         const ControlVec    *v)
{
    CONTROL_CHECK_CTX(ctx);
    CONTROL_CHECK_NOT_NULL(ctx, out && m && v);
    CONTROL_REQUIRE(
        ctx, m->cols == v->size, CONTROL_ERROR_DIMENSION_MISMATCH);

    size_t new_size = m->rows;

    ControlVec res = {0};
    CONTROL_TRY(__Control_Vec_CreateInArenaRaw(ctx, &res, ctx->scratch, new_size));

    res.size = new_size;

    for (size_t i = 0; i < new_size; i++)
    {
        float dot_prod = 0.0f;
        for (size_t j = 0; j < m->cols; j++)
        {
            dot_prod += m->data[i * m->cols + j] * v->coeffs[j];
        }

        res.coeffs[i] = dot_prod;
    }

    if (out->capacity < new_size)
    {
        CONTROL_TRY(__Control_Vec_CreateInArenaRaw(ctx, out, ctx->scratch, new_size));
    }

    out->size = new_size;
    for (size_t i = 0; i < new_size; i++)
    {
        out->coeffs[i] = res.coeffs[i];
    }

    return CONTROL_OK;
}

ControlResult
Control_Vec_Add(ControlHandle *ctx, ControlVec *out, const ControlVec *lhs, const ControlVec *rhs)
{
    CONTROL_CHECK_CTX(ctx);
    CONTROL_CHECK_NOT_NULL(ctx, out && lhs && rhs);
    CONTROL_REQUIRE(
        ctx, lhs->size == rhs->size, CONTROL_ERROR_DIMENSION_MISMATCH);

    size_t size = lhs->size;
    if (out->capacity < size)
    {
        CONTROL_TRY(__Control_Vec_CreateInArenaRaw(ctx, out, ctx->scratch, size));
    }

    out->size = size;

    for (size_t i = 0; i < size; i++)
    {
        float sum = 0.0f;
        if (i < lhs->size)
        {
            sum += lhs->coeffs[i];
        }
        if (i < rhs->size)
        {
            sum += rhs->coeffs[i];
        }

        out->coeffs[i] = sum;
    }

    return CONTROL_OK;
}

ControlResult
Control_Vec_Scale(ControlHandle *ctx, ControlVec *out, const ControlVec *v, float scalar)
{
    CONTROL_CHECK_CTX(ctx);
    CONTROL_CHECK_NOT_NULL(ctx, out && v);

    if (out->capacity < v->size)
    {
        CONTROL_TRY(__Control_Vec_CreateInArenaRaw(ctx, out, ctx->scratch, v->size));
    }

    out->size = v->size;

    for (size_t i = 0; i < v->size; i++)
    {
        out->coeffs[i] = v->coeffs[i] * scalar;
    }

    return CONTROL_OK;
}

bool Control_Matrix_IsValid(const ControlMatrix *m)
{
    return m != NULL && m->data != NULL;
}

bool Control_Vec_IsValid(const ControlVec *v)
{
    return v != NULL && v->coeffs != NULL;
}

/*
 * ----------------------------------------
 * router.c
 * ----------------------------------------
*/






void Control_StateSpace_StepContinuous(ControlHandle *ctx, ControlStateSpace *ss, float dt)
{
    CONTROL_UNUSED(ctx);

    if (ss->B.cols == 1 && ss->C.rows == 1)
    {
        __Control_StateSpace_StepSISO(ss, dt);
    }
    else
    {
        __Control_StateSpace_StepMIMO(ss, dt);
    }
}

/*
 * ----------------------------------------
 * mimo.c
 * ----------------------------------------
*/


#include <stddef.h>


#ifndef MAX_SYSTEM_ORDER
    #define MAX_SYSTEM_ORDER 10
#endif

void __Control_StateSpace_StepMIMO(ControlStateSpace *ss, float dt)
{
    size_t n = ss->A.rows; // num states  (n x n)
    size_t m = ss->B.cols; // num inputs  (n x m)
    size_t p = ss->C.rows; // num outputs (p x n)

    float x_dot[MAX_SYSTEM_ORDER] = {0.0f};

    // Output
    // y = Cx + Du
    for (size_t i = 0; i < p; i++)
    {
        ss->y.coeffs[i] = 0.0f;

        // C(p x n) * x (n x 1)
        for (size_t j = 0; j < n; j++)
        {
            ss->y.coeffs[i] += ss->C.data[i * n + j] * ss->x.coeffs[j];
        }

        // D(p x m) * u (m x 1)
        for (size_t j = 0; j < m; j++)
        {
            ss->y.coeffs[i] += ss->D.data[i * m + j] * ss->u.coeffs[j];
        }
    }

    // State
    for (size_t i = 0; i < n; i++)
    {
        x_dot[i] = 0.0f;

        // A(n x n) * x(n x 1)
        for (size_t j = 0; j < n; j++)
        {
            x_dot[i] += ss->A.data[i * n + j] * ss->x.coeffs[j];
        }

        // B(n x m) * u (m x 1)
        for (size_t j = 0; j < m; j++)
        {
            x_dot[i] += ss->B.data[i * n + j] * ss->u.coeffs[j];
        }
    }

    // Euler Integration
    for (size_t i = 0; i < n; i++)
    {
        ss->x.coeffs[i] += x_dot[i] * dt;
    }
}

#undef MAX_SYSTEM_ORDER

/*
 * ----------------------------------------
 * siso.c
 * ----------------------------------------
*/





#include <stddef.h>



static ControlResult __gen_sys_matrix_InPersistent(ControlHandle                 *ctx,
                                                   ControlSystemMatrix           *out,
                                                   const ControlTransferFunction *tf)
{
    size_t n = tf->den.size - 1;

    CONTROL_TRY(Control_Matrix_AllocPersistent(ctx, out, n, n));

    for (size_t i = 0; i < n - 1; i++)
    {
        out->data[i * n + (i + 1)] = 1;
    }

    for (size_t i = 0; i < n; i++)
    {
        out->data[(n - 1) * n + i] = -tf->den.coeffs[n - i];
    }

    return CONTROL_OK;
}

static ControlResult __gen_input_matrix_InPersistent(ControlHandle                 *ctx,
                                                     ControlInputMatrix            *out,
                                                     const ControlTransferFunction *tf)
{
    size_t n = tf->den.size - 1;
    CONTROL_TRY(Control_Matrix_AllocPersistent(ctx, out, n, 1));
    out->data[n - 1] = 1;
    return CONTROL_OK;
}

static ControlResult __gen_output_matrix_InPersistent(ControlHandle                 *ctx,
                                                      ControlOutputMatrix           *out,
                                                      const ControlTransferFunction *tf)
{
    size_t n = tf->den.size - 1;
    CONTROL_TRY(Control_Matrix_AllocPersistent(ctx, out, 1, n));

    size_t m = tf->num.size - 1;

    float b0 = 0.0f;
    if (tf->num.size == tf->den.size)
    {
        b0 = tf->num.coeffs[0];
    }

    size_t offset = n - m;

    for (size_t i = 0; i < n; i++)
    {
        size_t k = n - i; // Polynomial order

        float ak = tf->den.coeffs[k];
        float bk = 0.0f;

        if (k >= offset)
        {
            bk = tf->num.coeffs[k - offset];
        }
        out->data[i] = bk - ak * b0;
    }
    return CONTROL_OK;
}

static ControlResult __gen_feedthrough_matrix_InPersistent(ControlHandle                 *ctx,
                                                           ControlFeedbackMatrix         *out,
                                                           const ControlTransferFunction *tf)
{
    CONTROL_TRY(Control_Matrix_AllocPersistent(ctx, out, 1, 1));

    float b0 = 0.0f;
    if (tf->num.size == tf->den.size)
    {
        b0 = tf->num.coeffs[0];
    }

    out->data[0] = b0;
    return CONTROL_OK;
}

ControlResult Control_StateSpace_FromTF(ControlHandle                 *ctx,
                                        ControlStateSpace             *out,
                                        const ControlTransferFunction *tf)
{
    CONTROL_CHECK_CTX(ctx);
    CONTROL_CHECK_NOT_NULL(ctx, out && tf);
    CONTROL_TRY(__gen_sys_matrix_InPersistent(ctx, &out->A, tf));
    CONTROL_TRY(__gen_input_matrix_InPersistent(ctx, &out->B, tf));
    CONTROL_TRY(__gen_output_matrix_InPersistent(ctx, &out->C, tf));
    CONTROL_TRY(__gen_feedthrough_matrix_InPersistent(ctx, &out->D, tf));

    return CONTROL_OK;
}

#ifndef MAX_SYSTEM_ORDER
    #define MAX_SYSTEM_ORDER 10
#endif

void __Control_StateSpace_StepSISO(ControlStateSpace *ss, float dt)
{
    size_t n = ss->A.rows;

    float x_dot[MAX_SYSTEM_ORDER] = {0.0f};

    // Step 1:
    // y = Cx + Du

    ss->y.coeffs[0] = 0.0f;

    // Cx calculations
    for (size_t i = 0; i < n; i++)
    {
        ss->y.coeffs[0] += ss->C.data[i] * ss->x.coeffs[i];
    }

    // Du calculation
    ss->y.coeffs[0] += ss->D.data[0] * ss->u.coeffs[0];

    // Step 2:
    // x_dot = Ax + Bu
    for (size_t i = 0; i < n; i++)
    {
        x_dot[i] = 0.0f;

        for (size_t j = 0; j < n; j++)
        {
            x_dot[i] += ss->A.data[i * n + j] * ss->x.coeffs[j];
        }

        x_dot[i] += ss->B.data[i] * ss->u.coeffs[0];
    }

    // Euler Integration
    for (size_t i = 0; i < n; i++)
    {
        ss->x.coeffs[i] += x_dot[i] * dt;
    }
}

#undef MAX_SYSTEM_ORDER

/*
 * ----------------------------------------
 * tf.c
 * ----------------------------------------
*/





#include <stdbool.h>
#include <stddef.h>


#define REQUIRE_VALID_TF(ctx, tf_ptr, msg)                                                         \
    CONTROL_REQUIRE(ctx, Control_TF_IsValid(tf_ptr), CONTROL_ERROR_INVALID_ARGUMENT, msg)

CONTROLLY_PRIVATE_API ControlResult __Control_Vec_CreateInArena(ControlHandle *ctx,
                                                                ControlVec    *out,
                                                                ControlArena  *a,
                                                                size_t         capacity)
{
    out->capacity = 0;
    out->size     = 0;
    out->coeffs   = NULL;

    out->coeffs = (float *)Control_Arena_Alloc(a, capacity * sizeof(float));
    CONTROL_REQUIRE(ctx, out->coeffs, CONTROL_ERROR_OUT_OF_MEMORY);

    out->capacity = capacity;
    return CONTROL_OK;
}

static ControlResult __Control_Poly_CreateInArena(
    ControlHandle *ctx, ControlVec *out, ControlArena *a, const float *coeffs, size_t size)
{
    CONTROL_TRY(__Control_Vec_CreateInArena(ctx, out, a, size));

    out->size = size;

    for (size_t i = 0; i < size; i++)
    {
        out->coeffs[i] = coeffs[i];
    }

    return CONTROL_OK;
}

ControlResult Control_Poly_Canonicalize(ControlHandle *ctx, ControlVec *out, const ControlVec *v)
{
    CONTROL_CHECK_CTX(ctx);
    CONTROL_CHECK_NOT_NULL(ctx, out && v);

    size_t i = 0;

    // TODO: Compare with epsilon
    while (i + 1 < v->size && v->coeffs[i] == 0.0f)
    {
        i++;
    }

    size_t new_capacity = v->capacity - i;

    out->coeffs   = &v->coeffs[i];
    out->size     = v->size - i;
    out->capacity = new_capacity;

    return CONTROL_OK;
}

ControlResult
Control_Poly_AllocScratch(ControlHandle *ctx, ControlVec *out, const float *coeffs, size_t size)
{
    CONTROL_CHECK_CTX(ctx);
    CONTROL_CHECK_NOT_NULL(ctx, out && coeffs);

    return __Control_Poly_CreateInArena(ctx, out, ctx->scratch, coeffs, size);
}

ControlResult
Control_Poly_AllocPersistent(ControlHandle *ctx, ControlVec *out, const float *coeffs, size_t size)
{
    CONTROL_CHECK_CTX(ctx);
    CONTROL_CHECK_NOT_NULL(ctx, out && coeffs);

    return __Control_Poly_CreateInArena(ctx, out, ctx->persistent, coeffs, size);
}

ControlResult
Control_Poly_Add(ControlHandle *ctx, ControlVec *out, const ControlVec *a, const ControlVec *b)
{
    CONTROL_CHECK_CTX(ctx);
    CONTROL_CHECK_NOT_NULL(ctx, out && a && b);

    size_t max_size = a->size > b->size ? a->size : b->size;

    if (out->coeffs == NULL || out->capacity < max_size)
    {
        CONTROL_TRY(__Control_Vec_CreateInArena(ctx, out, ctx->scratch, max_size));
    }

    size_t a_offset = max_size - a->size;
    size_t b_offset = max_size - b->size;

    for (size_t i = 0; i < max_size; i++)
    {
        float sum = 0.0f;
        if (i >= a_offset)
        {
            sum += a->coeffs[i - a_offset];
        }
        if (i >= b_offset)
        {
            sum += b->coeffs[i - b_offset];
        }

        out->coeffs[i] = sum;
    }

    out->size = max_size;
    return CONTROL_OK;
}

ControlResult
Control_Poly_Multiply(ControlHandle *ctx, ControlVec *out, const ControlVec *a, const ControlVec *b)
{
    CONTROL_CHECK_CTX(ctx);
    CONTROL_CHECK_NOT_NULL(ctx, out && a && b);

    size_t new_size = a->size + b->size - 1;
    if (out->coeffs == NULL || out->capacity < new_size)
    {
        CONTROL_TRY(__Control_Vec_CreateInArena(ctx, out, ctx->scratch, new_size));
    }

    out->size = new_size;

    for (size_t i = 0; i < new_size; i++)
    {
        out->coeffs[i] = 0.0f;
    }

    for (size_t i = 0; i < a->size; i++)
    {
        for (size_t j = 0; j < b->size; j++)
        {
            out->coeffs[i + j] += a->coeffs[i] * b->coeffs[j];
        }
    }

    return CONTROL_OK;
}

ControlResult Control_TF_FromPoly(ControlHandle           *ctx,
                                  ControlTransferFunction *out,
                                  const ControlVec        *num,
                                  const ControlVec        *dem)
{
    CONTROL_CHECK_CTX(ctx);
    CONTROL_CHECK_NOT_NULL(ctx, out && num && dem);

    // TODO: Check if we can use allocated memory if abailable
    out->num = *num;
    out->den = *dem;
    return CONTROL_OK;
}

ControlResult Control_TF_Multiply(ControlHandle                 *ctx,
                                  ControlTransferFunction       *out,
                                  const ControlTransferFunction *G1,
                                  const ControlTransferFunction *G2)
{
    CONTROL_CHECK_CTX(ctx);
    CONTROL_CHECK_NOT_NULL(ctx, out);

    CONTROL_TRY(Control_TF_Validate(ctx, G1));
    CONTROL_TRY(Control_TF_Validate(ctx, G2));

    ControlVec convolved_num = {0};
    ControlVec convolved_den = {0};

    CONTROL_TRY(Control_Poly_Multiply(ctx, &convolved_num, &G1->num, &G2->num));
    CONTROL_TRY(Control_Poly_Multiply(ctx, &convolved_den, &G1->den, &G2->den));

    CONTROL_TRY(Control_Poly_Canonicalize(ctx, &convolved_num, &convolved_num));
    CONTROL_TRY(Control_Poly_Canonicalize(ctx, &convolved_den, &convolved_den));

    CONTROL_TRY(Control_TF_FromPoly(ctx, out, &convolved_num, &convolved_den));

    return CONTROL_OK;
}

inline bool Control_TF_IsValid(const ControlTransferFunction *tf)
{
    return tf && tf->den.coeffs && tf->num.coeffs && tf->den.size != 0 && tf->den.coeffs[0] != 0;
}

ControlResult Control_TF_Validate(ControlHandle *ctx, const ControlTransferFunction *tf)
{
    CONTROL_CHECK_CTX(ctx);
    CONTROL_CHECK_NOT_NULL(ctx, tf);

    CONTROL_REQUIRE(ctx,
                    tf->num.coeffs && tf->den.coeffs,
                    CONTROL_ERROR_INVALID_ARGUMENT);

    CONTROL_REQUIRE(ctx,
                    tf->den.size != 0 && tf->den.coeffs[0] != 0.0f,
                    CONTROL_ERROR_DIVIDE_BY_ZERO);

    return CONTROL_OK;
}

ControlResult Control_TF_ClosedLoop(ControlHandle                 *ctx,
                                    ControlTransferFunction       *out,
                                    const ControlTransferFunction *G,
                                    float                          gain,
                                    ControlFeedbackType            unity)
{
    CONTROL_CHECK_CTX(ctx);
    CONTROL_CHECK_NOT_NULL(ctx, out);
    CONTROL_TRY(Control_TF_Validate(ctx, G));

    /*
     * G(s) = N(s)/D(s)
     *
     * H(s) = G(s)/(1 +/- k*G(s)) = (N(s)/D(s))/(1 +/- k*(N(s)/D(s))
     * H(s) = N(s)/(D(s) +/- k*N(s))
     */

    if (unity == TF_FEEDBACK_POSITIVE)
    {
        gain = -gain;
    }

    ControlVec scaled_num = {0};
    if (gain != 1.0f)
    {
        CONTROL_TRY(__Control_Vec_CreateInArena(ctx, &scaled_num, ctx->scratch, G->num.size));
        for (size_t i = 0; i < G->num.size; i++)
        {
            scaled_num.coeffs[i] = G->num.coeffs[i] * gain;
        }
    }
    else
    {
        scaled_num = G->num;
    }

    ControlVec denom = {0};
    CONTROL_TRY(Control_Poly_Add(ctx, &denom, &G->den, &scaled_num));
    CONTROL_TRY(Control_TF_FromPoly(ctx, out, &G->num, &denom));

    return CONTROL_OK;
}

ControlResult Control_TF_Persist(ControlHandle                 *ctx,
                                 ControlTransferFunction       *out,
                                 const ControlTransferFunction *tf)
{
    CONTROL_CHECK_CTX(ctx);
    CONTROL_CHECK_NOT_NULL(ctx, out);

    // NOTE: We do not validate, invalid state can be copied
    // CONTROL_TRY(Control_TF_Validate(ctx, tf));

    CONTROL_TRY(__Control_Poly_CreateInArena(
        ctx, &out->num, ctx->persistent, tf->num.coeffs, tf->num.size));
    CONTROL_TRY(__Control_Poly_CreateInArena(
        ctx, &out->den, ctx->persistent, tf->den.coeffs, tf->den.size));

    return CONTROL_OK;
}

/*
 * ----------------------------------------
 * pid.c
 * ----------------------------------------
*/





#include <stdbool.h>
#include <stddef.h>


void Control_PID_Init(
    ControlPIDController *pid, float kp, float ki, float kd, const ControlPIDConfig *config)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->integral   = 0.0f;
    pid->prev_error = 0.0f;

    if (config != NULL)
    {
        pid->config = *config;
    }
    else
    {
        pid->config.enable_anti_windup = false;
        pid->config.max_out            = 0.0f;
        pid->config.min_out            = 0.0f;
    }
}

void Control_PID_Reset(ControlPIDController *pid)
{
    pid->integral   = 0.0f;
    pid->prev_error = 0.0f;
}

float Control_PID_Update(ControlPIDController *pid, float setpoint, float measurement, float dt)
{
    if (dt <= 0.0f)
    {
        return 0.0f;
    }

    float error = setpoint - measurement;

    float p_term = pid->kp * error;

    pid->integral += error * dt;

    float i_term = pid->ki * pid->integral;

    float derivative = (error - pid->prev_error) / dt;
    float d_term     = pid->kd * derivative;

    float output = p_term + i_term + d_term;

    if (pid->config.enable_anti_windup)
    {
        if (output > pid->config.max_out)
        {
            output = pid->config.max_out;
            // Undo integral operation to reduce windup
            pid->integral -= error * dt;
        }
        else if (output < pid->config.min_out)
        {
            output = pid->config.min_out;
            pid->integral -= error * dt;
        }
    }

    pid->prev_error = error;

    return output;
}

ControlResult
Control_PID_ToTF(ControlHandle *ctx, ControlTransferFunction *out, const ControlPIDController *pid)
{
    float num[3] = {pid->kd, pid->kp, pid->ki};
    float den[3] = {1.0f, 0.0f};

    ControlVec num_v = {0};
    ControlVec den_v = {0};

    CONTROL_TRY(Control_Poly_AllocPersistent(ctx, &num_v, num, 3));
    CONTROL_TRY(Control_Poly_AllocPersistent(ctx, &den_v, den, 3));

    CONTROL_TRY(Control_TF_FromPoly(ctx, out, &num_v, &den_v));

    return CONTROL_OK;
}

#undef CONTROL_UNUSED
#undef CONTROL_ALIGN_UP
#undef CONTROL_THROW
#undef CONTROL_REQUIRE
#undef CONTROL_CHECK_CTX
#undef CONTROL_CHECK_OUT
#undef CONTROL_CHECK_NOT_NULL
#undef CONTROL_TRY

#endif /* CONTROLLY_IMPLEMENTATION */
