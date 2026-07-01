#include <ccontrol/tf.h>
#include <stdint.h>
#include <unity.h>
#include <unity_fixture.h>

static void runAllTests(void) { RUN_TEST_GROUP(PolyMath); }

int main(int argc, const char *argv[])
{
    return UnityMain(argc, argv, runAllTests);
}
