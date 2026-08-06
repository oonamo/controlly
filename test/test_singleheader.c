#include "unity.h"

#ifdef HEADERLIB_TEST
    #include "controlly.h"
#endif

#include <unity_fixture.h>

TEST_GROUP(SingleHeader);

TEST_SETUP(SingleHeader)
{
}

TEST_TEAR_DOWN(SingleHeader)
{
}

TEST(SingleHeader, InternalMacrosAreNotAvailable)
{
#ifndef HEADERLIB_TEST
    TEST_IGNORE_MESSAGE("Modular build - Skipping single-header macro leaking test");
#endif
#if defined(CONTROL_UNUSED)
    TEST_FAIL_MESSAGE("Leaked CONTROL_UNUSED macro");
#endif
#if defined(CONTROL_ALIGN_UP)
    TEST_FAIL_MESSAGE("Leaked CONTROL_ALIGN_UP macro");
#endif
#if defined(CONTROL_THROW)
    TEST_FAIL_MESSAGE("Leaked CONTROL_THROW macro");
#endif
#if defined(CONTROL_REQUIRE)
    TEST_FAIL_MESSAGE("Leaked CONTROL_REQUIRE macro");
#endif
#if defined(CONTROL_CHECK_CTX)
    TEST_FAIL_MESSAGE("Leaked CONTROL_CHECK_CTX macro");
#endif
#if defined(CONTROL_CHECK_OUT)
    TEST_FAIL_MESSAGE("Leaked CONTROL_CHECK_OUT macro");
#endif
#if defined(CONTROL_CHECK_NOT_NULL)
    TEST_FAIL_MESSAGE("Leaked CONTROL_CHECK_NOT_NULL macro");
#endif
#if defined(CONTROL_TRY)
    TEST_FAIL_MESSAGE("Leaked CONTROL_TRY macro");
#endif

    TEST_PASS_MESSAGE("No internal macros were leaked");
}

TEST_GROUP_RUNNER(SingleHeader)
{
    RUN_TEST_CASE(SingleHeader, InternalMacrosAreNotAvailable);
}
