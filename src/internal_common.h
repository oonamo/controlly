#ifndef _INTERNAL_COMMON
#define _INTERNAL_COMMON

#define CCONTROL_UNUSED(x) (void)(x)

#ifndef CCONTROL_ARENA_ALIGN_SIZE
#define CCONTROL_ARENA_ALIGN_SIZE 8
#endif

#define ALIGN_UP(size, align) (((size_t)(size) + ((size_t)(align) - 1)) & ~((size_t)(align) - 1))

#endif
