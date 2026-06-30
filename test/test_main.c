#include <unity.h>
#include <unity_fixture.h>

TEST_GROUP(MAIN);

TEST_SETUP(MAIN) {}
TEST_TEAR_DOWN(MAIN) {}

TEST(MAIN, PolynomialMultiplication) {}

TEST_GROUP_RUNNER(MAIN) { RUN_TEST_CASE(MAIN, PolynomialMultiplication) }

static void runAllTests(void) { RUN_TEST_GROUP(MAIN); }

int main(int argc, const char *argv[])
{
    return UnityMain(argc, argv, runAllTests);
}
