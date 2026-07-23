#ifndef _INTERNAL_COMMON
#define _INTERNAL_COMMON

#define CCONTROL_UNUSED(x)    (void)(x)

#define ALIGN_UP(size, align) (((size_t)(size) + ((size_t)(align) - 1)) & ~((size_t)(align) - 1))

#define CCONTROL_THROW(ctx, code, msg)                                                             \
    do                                                                                             \
    {                                                                                              \
        if ((ctx) && (ctx)->on_error)                                                              \
        {                                                                                          \
            (ctx)->on_error((code), (msg), (ctx)->user_data);                                      \
        }                                                                                          \
    } while (0)

#define CCONTROL_REQUIRE(ctx, condition, err_code, msg)                                            \
    do                                                                                             \
    {                                                                                              \
        if (!(condition))                                                                          \
        {                                                                                          \
            CCONTROL_THROW(ctx, err_code, msg);                                                    \
            return (err_code);                                                                     \
        }                                                                                          \
    } while (0)

#define CHECK_CTX(ctx)                                                                             \
    CCONTROL_REQUIRE((ctx), (ctx), CCONTROL_ERROR_CTX_UNINITIALIZED, "Context was unintialized");

#define CHECK_OUT(ctx, out)                                                                        \
    CCONTROL_REQUIRE((ctx), (out), CCONTROL_ERROR_NULL_PTR, "Out parameter was NULL")

#define CHECK_PTR(ctx, ptr, msg) CCONTROL_REQUIRE((ctx), (ptr), CCONTROL_ERROR_NULL_PTR, msg)

#define CHECK_NOT_NULL(ctx, ptrs_cond, msg)                                                        \
    CCONTROL_REQUIRE((ctx), (ptrs_cond), CCONTROL_ERROR_NULL_PTR, msg)

#define CCONTROL_TRY(expr)                                                                         \
    do                                                                                             \
    {                                                                                              \
        ControlResult _ccontrol_status_try = (expr);                                               \
        if (_ccontrol_status_try != CCONTROL_OK)                                                   \
        {                                                                                          \
            return _ccontrol_status_try;                                                           \
        }                                                                                          \
                                                                                                   \
    } while (0)

#endif
