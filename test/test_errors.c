#include <controlly/core.h>
#include <unity_fixture.h>

#include "../src/internal_common.h"
#include "unity.h"

TEST_GROUP(ErrorHandler);

static ControlResult captured_code;
static const char   *captured_msg;
static const char   *captured_verbose_data;
static void         *captured_user_data;

static void
MockErrorHandler(ControlResult code, const char *msg, const char *verbose_msg, void *user_data)
{
    captured_code         = code;
    captured_msg          = msg;
    captured_verbose_data = verbose_msg;
    captured_user_data    = user_data;
}

TEST_SETUP(ErrorHandler)
{
    captured_code         = CONTROL_OK;
    captured_msg          = NULL;
    captured_verbose_data = NULL;
    captured_user_data    = NULL;
}

TEST_TEAR_DOWN(ErrorHandler)
{
}

static void MockThrowableFunction(ControlHandle *ctx, ControlResult throw_code)
{
    CONTROL_THROW(ctx, throw_code);
}

TEST(ErrorHandler, ErrorIsCaughtCorrectly)
{
    ControlHandle ctx = {0};
    ctx.on_error      = MockErrorHandler;
    MockThrowableFunction(&ctx, CONTROL_ERROR_NULL_PTR);

    TEST_ASSERT_EQUAL(CONTROL_ERROR_NULL_PTR, captured_code);
    TEST_ASSERT_EQUAL_STRING(CONTROLLY_ERROR_NULL_PTR_MSG, captured_msg);
}

TEST(ErrorHandler, ProgramDoesNotCrashWithoutHandler)
{
    ControlHandle ctx = {0};
    MockThrowableFunction(&ctx, CONTROL_ERROR_OUT_OF_MEMORY);
}

TEST(ErrorHandler, NullContextDoesNotCrash)
{
    MockThrowableFunction(NULL, CONTROL_ERROR_OUT_OF_MEMORY);
}

TEST(ErrorHandler, DefaultVerboseErrorPassesCorrectContext)
{
#ifndef CONTROLLY_VERBOSE_ERRORS
    TEST_IGNORE_MESSAGE("Verbose errors are disabled - skipping");
#endif

    ControlHandle ctx = {0};
    ctx.on_error      = MockErrorHandler;

    MockThrowableFunction(&ctx, CONTROL_ERROR_NULL_PTR);

    TEST_ASSERT_EQUAL_STRING("MockThrowableFunction", captured_verbose_data);
}

TEST(ErrorHandler, UserDataIsForwaredCorrectly)
{
    // Check if this id is perserved, useful for identifying context
    int dummy_thread_id = 32;

    ControlHandle ctx = {0};
    ctx.on_error      = MockErrorHandler;
    ctx.user_data     = &dummy_thread_id;

    MockThrowableFunction(&ctx, CONTROL_ERROR_INVALID_ARGUMENT);

    TEST_ASSERT_EQUAL_PTR(&dummy_thread_id, captured_user_data);
    TEST_ASSERT_EQUAL(dummy_thread_id, *(int *)captured_user_data);
}

TEST(ErrorHandler, VerboseDisabledPassesNULLContext)
{
#ifdef CONTROLLY_VERBOSE_ERRORS
    TEST_IGNORE_MESSAGE("Verbose errors are enabled - skipping");
#endif

    ControlHandle ctx = {0};
    ctx.on_error      = MockErrorHandler;

    MockThrowableFunction(&ctx, CONTROL_ERROR_OUT_OF_MEMORY);

    TEST_ASSERT_NULL(captured_verbose_data);
}

TEST_GROUP_RUNNER(ErrorHandler)
{
    RUN_TEST_CASE(ErrorHandler, ErrorIsCaughtCorrectly);
    RUN_TEST_CASE(ErrorHandler, ProgramDoesNotCrashWithoutHandler);
    RUN_TEST_CASE(ErrorHandler, NullContextDoesNotCrash);
    RUN_TEST_CASE(ErrorHandler, DefaultVerboseErrorPassesCorrectContext);
    RUN_TEST_CASE(ErrorHandler, UserDataIsForwaredCorrectly);
    RUN_TEST_CASE(ErrorHandler, VerboseDisabledPassesNULLContext);
}
