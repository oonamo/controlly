#include "src/tf.h"
#include "stdio.h"

int main()
{
    ControlSystemInit();
    float a[3] = {1, 2, 3};
    float b[2] = {1, 2};

    control_vector_t a_coeffs = PolyCoeffVector(a, 3);
    control_vector_t b_coeffs = PolyCoeffVector(b, 2);

    TransferFunction Gp = TransferFunctionFromCoeffs(a_coeffs, b_coeffs);
    TransferFunction Gc = TransferFunctionFromCoeffs(a_coeffs, b_coeffs);

    char num_buffer[64];
    char dem_buffer[64];
    PolyCoeffVectorToStr(&Gp.num, 's', num_buffer, 64);
    PolyCoeffVectorToStr(&Gp.dem, 's', dem_buffer, 64);
    printf("(%s)/(%s)\n", num_buffer, dem_buffer);

    char buffer[64];
    TransferFunction ForwardGain = MultiplyTransferFunctrions(&Gp, &Gc);
    TransferFunctionToStr(&ForwardGain, 's', buffer, 64);

    printf("Forward Gain: %s\n", buffer);

    return 0;
}
