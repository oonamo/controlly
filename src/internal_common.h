#ifndef _INTERNAL_COMMON
#define _INTERNAL_COMMON

#define CONTROL_UNUSED(x) (void)(x)

#define ALIGN_UP(size, align) (((size_t)(size) + ((size_t)(align) - 1)) & ~((size_t)(align) - 1))

#define CONTROL_THROW(ctx, code, msg)                                                             \
    do                                                                                             \
    {                                                                                              \
        if ((ctx) && (ctx)->on_error)                                                              \
        {                                                                                          \
            (ctx)->on_error((code), (msg), (ctx)->user_data);                                      \
        }                                                                                          \
    } while (0)

#define CONTROL_REQUIRE(ctx, condition, err_code, msg)                                            \
    do                                                                                             \
    {                                                                                              \
        if (!(condition))                                                                          \
        {                                                                                          \
            CONTROL_THROW(ctx, err_code, msg);                                                    \
            return (err_code);                                                                     \
        }                                                                                          \
    } while (0)

#define CHECK_CTX(ctx)                                                                             \
    CONTROL_REQUIRE((ctx), (ctx), CONTROL_ERROR_CTX_UNINITIALIZED, "Context was unintialized");

#define CHECK_OUT(ctx, out)                                                                        \
    CONTROL_REQUIRE((ctx), (out), CONTROL_ERROR_NULL_PTR, "Out parameter was NULL")

#define CHECK_NOT_NULL(ctx, ptr, msg) CONTROL_REQUIRE((ctx), (ptr), CONTROL_ERROR_NULL_PTR, msg)

#define CONTROL_TRY(expr)                                                                         \
    do                                                                                             \
    {                                                                                              \
        ControlResult _controlly_status_try = (expr);                                               \
        if (_controlly_status_try != CONTROL_OK)                                                   \
        {                                                                                          \
            return _controlly_status_try;                                                           \
        }                                                                                          \
                                                                                                   \
    } while (0)

#endif
