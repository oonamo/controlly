/**
 * @file core.h
 * @brief Core module of Controlly
 *
 * This module contains the core definitions needed for the Controlly library.
 * It exposes error codes, memory management, and the ControlHandle for handling library operations
 */
#ifndef _CONTROLLY_CORE_H
#define _CONTROLLY_CORE_H

#include <controlly/arena.h>
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

#ifndef CONTROLLY_ERROR_OK_MSG
    #define CONTROLLY_ERROR_OK_MSG "Operation success"
#endif
#ifndef CONTROLLY_ERROR_CTX_UNINITIALIZED_MSG
    #define CONTROLLY_ERROR_CTX_UNINITIALIZED_MSG "Context was unintialized"
#endif
#ifndef CONTROLLY_ERROR_OUT_OF_MEMORY_MSG
    #define CONTROLLY_ERROR_OUT_OF_MEMORY_MSG "Out of memory"
#endif
#ifndef CONTROLLY_ERROR_DIVIDE_BY_ZERO_MSG
    #define CONTROLLY_ERROR_DIVIDE_BY_ZERO_MSG "Division by zero"
#endif
#ifndef CONTROLLY_ERROR_INVALID_ARGUMENT_MSG
    #define CONTROLLY_ERROR_INVALID_ARGUMENT_MSG "An invalid argument was passed"
#endif
#ifndef CONTROLLY_ERROR_NULL_PTR_MSG
    #define CONTROLLY_ERROR_NULL_PTR_MSG "A null pointer was passed"
#endif
#ifndef CONTROLLY_ERROR_FEATURE_DISABLED_MSG
    #define CONTROLLY_ERROR_FEATURE_DISABLED_MSG "Disabled feature was used"
#endif
#ifndef CONTROLLY_ERROR_DIMENSION_MISMATCH_MSG
    #define CONTROLLY_ERROR_DIMENSION_MISMATCH_MSG "Dimensions are mismatched"
#endif
#ifndef CONTROLLY_ERROR_UNKNOWN_MSG
    #define CONTROLLY_ERROR_UNKNOWN_MSG "An unknown error occured"
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

/*
 * @brief Structure for Controlly context
 *
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

#endif // _CONTROLLY_CORE_H
