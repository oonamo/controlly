/**
 * @file matrix.h
 * @brief Minimal vector and matrix math implementations
 *
 * This module contains a minimal support of common linear algebra functions for use with the
 * CControl library.
 */
#ifndef _CCONTROL_MATRIX_H
#define _CCONTROL_MATRIX_H

#include <ccontrol/core.h>
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
#define CCONTROL_EMPTY_VEC ((ControlVec){0})

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
#define CCONTROL_EMPTY_MATRIX ((ControlMatrix){0})

/**
 * @brief Allocates a ControlVec in a scratch arena.
 *
 * @note Allocates from the `ctx->scratch` arena.
 *
 * @param[in,out] ctx  Pointer to the initialized control context.
 * @param[out]    out  Pointer to the the destination ControlVec
 * @param[in]     size Amount of elements to allocate
 *
 * @return CCONTROL_OK on success.
 * @return CCONTROL_ERROR_OUT_OF_MEMORY if the scratch arena is exhausted.
 * @return CCONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 * @return CCONTROL_ERROR_NULL_PTR if the out vector is NULL.
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
 * @return CCONTROL_OK on success.
 * @return CCONTROL_ERROR_OUT_OF_MEMORY if the scratch persistent is exhausted.
 * @return CCONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 * @return CCONTROL_ERROR_NULL_PTR if the out vector is NULL.
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
 * @return CCONTROL_OK on success.
 * @return CCONTROL_ERROR_OUT_OF_MEMORY if the scratch persistent is exhausted.
 * @return CCONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 * @return CCONTROL_ERROR_NULL_PTR if the out vector is NULL.
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
 * @return CCONTROL_OK on success.
 * @return CCONTROL_ERROR_OUT_OF_MEMORY if the scratch persistent is exhausted.
 * @return CCONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 * @return CCONTROL_ERROR_NULL_PTR if the out vector is NULL.
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
 * @return CCONTROL_OK on success.
 * @return CCONTROL_ERROR_OUT_OF_MEMORY if the scratch scratch is exhausted.
 * @return CCONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 * @return CCONTROL_ERROR_NULL_PTR if the out vector is NULL.
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
 * @return CCONTROL_OK on success.
 * @return CCONTROL_ERROR_OUT_OF_MEMORY if the scratch persistent is exhausted.
 * @return CCONTROL_ERROR_CTX_UNINITIALIZED if the system context is uninitialized.
 * @return CCONTROL_ERROR_NULL_PTR if the out vector is NULL.
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
 * @return CCONTROL_OK on success.
 * @return CCONTROL_ERROR_DIMENSION_MISMATCH if m->cols != v->size.
 * @return CCONTROL_ERROR_NULL_PTR if any pointer is NULL.
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
 * @return CCONTROL_OK on success.
 * @return CCONTROL_ERROR_DIMENSION_MISMATCH if lhs->size != rhs->size.
 * @return CCONTROL_ERROR_NULL_PTR if any pointer is NULL.
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
 * @return CCONTROL_OK on success.
 * @return CCONTROL_ERROR_NULL_PTR if any pointer is NULL.
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

#endif // _CCONTROL_MATRIX_H
