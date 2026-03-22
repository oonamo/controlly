#include "src/arena.h"
#include "src/tf.h"
#include "stdio.h"
#include "stdlib.h"

int main()
{
    ControlSystemInit();
    float nums[2] = {-1.32, -2.23};

    control_vector_t v = PolyCoeffVector(nums, 2);

    char test[400];
    PolyCoeffVectorToStr(&v, 's', test, 320);

    printf("v = %s!\n", test);
    return 0;
}
