#ifndef _INTERNAL_COMMON
#define _INTERNAL_COMMON

#define CCONTROL_UNUSED(x) (void)(x)

#ifndef CCONTROL_ARENA_ALIGN_SIZE
#define CCONTROL_ARENA_ALIGN_SIZE 8
#endif

#define ALIGN_UP(size, align) (((size_t)(size) + ((size_t)(align) - 1)) & ~((size_t)(align) - 1))

#define CCONTROL_THROW(ctx, code, msg)                                                             \
    do                                                                                             \
    {                                                                                              \
        if ((ctx) && (ctx)->on_error)                                                              \
        {                                                                                          \
            (ctx)->on_error((code), (msg), (ctx)->user_data);                                      \
        }                                                                                          \
    } while (0)

#define CCONTROL_REQUIRE(ctx, condition, err_code, msg, retval)                                    \
    do                                                                                             \
    {                                                                                              \
        if (!(condition))                                                                          \
        {                                                                                          \
            CCONTROL_THROW(ctx, err_code, msg);                                                    \
            return retval;                                                                         \
        }                                                                                          \
    } while (0)

#endif
