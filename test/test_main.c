#include <ccontrol/tf.h>
#include <stdint.h>
#include <unity.h>
#include <unity_fixture.h>

static void runAllTests(void)
{
    RUN_TEST_GROUP(PolyMath);
    RUN_TEST_GROUP(TransferFunction);
    RUN_TEST_GROUP(GeneratedFuzz);
    RUN_TEST_GROUP(StateSpace);
}

int main(int argc, const char *argv[])
{
    return UnityMain(argc, argv, runAllTests);
}
