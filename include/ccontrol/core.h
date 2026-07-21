#ifndef CCONTROL_ERROR_H
#define CCONTROL_ERROR_H

#include "arena.h"
#include <stddef.h>

/**
 * Results of Control system operations
 */
typedef enum
{
    CCONTROL_OK = 0,                    /*!< Ok */
    CCONTROL_ERROR_OUT_OF_MEMORY,       /*!< Out of memory (arena is full) */
    CCONTROL_ERROR_DIVIDE_BY_ZERO,      /*!< A division by zero was attempted */
    CCONTROL_ERROR_INVALID_ARGUMENT,    /*!< An invalid parameter was passed to a function */
    CCONTROL_ERROR_USE_DISABLE_FEATURE, /*!< A disabled feature was used */
} ControlResult;

/*
 * Error callback with context
 */
typedef void (*ControlErrorCallback)(ControlResult code, const char *message, void *user_data);

/*
 * @brief Structure for CControl context
 *
 */
typedef struct
{
    ControlArena *persistent; /*!< Arena that holds permenant data */
    ControlArena *scratch;    /*!< Arena that holds temporary data */

    ControlErrorCallback
        on_error;    /*!< Calls the on_error function with context if an error occurs */
    void *user_data; /*!< Unique data for tracking contexts */
} ControlHandle;

/*
 * @brief Initializes the control system
 *
 * @param ctx Pointer to ControlHandle to initialize
 * @param p Pointer to Persistent ControlArena
 * @param s Pointer to Scratch ControlArena
 */
void Control_System_Init(ControlHandle *ctx, ControlArena *p, ControlArena *s);

/*
 * @brief Deinitiazlies a ControlHandle
 *
 * @note By default, the function does not attempt to free any allocated data
 */
void Control_System_DeInit(ControlHandle *ctx);

#endif
