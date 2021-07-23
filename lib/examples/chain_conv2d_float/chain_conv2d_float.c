#include <stdio.h>
#include "gpgpu_gles.h"

#define HEIGHT 4
#define WIDTH HEIGHT

int main()
{
    if (gpgpu_init(HEIGHT, WIDTH) != 0)
    {
        printf("Could not initialize the API\n");
        return 0;
    }

    float* a1 = malloc(WIDTH * HEIGHT * sizeof(float));
    float* res = malloc(WIDTH * HEIGHT * sizeof(float));

    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        a1[i] = i;
    }

    printf("Data before computation: \n");
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        printf("%.1f ", a1[i]);
        if ((i + 1) % WIDTH == 0)
            printf("\n");
    }
    printf("\n");

    float kernel[9] = {
        1.0, 1.0, 1.0,
        1.0, 1.0, 1.0,
        1.0, 1.0, 1.0,
    };

    // construct the computation chain
    EOperation ops[] = { FIR_CONV2D_FLOAT, FIR_CONV2D_FLOAT, FIR_CONV2D_FLOAT, FIR_CONV2D_FLOAT};
    UOperationPayloadFloat* payload = malloc(4 * sizeof(UOperationPayloadFloat)); // double the size for conv2d
    payload[0].arr = kernel;
    payload[1].n = 3;
    payload[2].arr = kernel;
    payload[3].n = 3;
    if (gpgpu_chain_apply_float(ops, payload, 4, a1, res) != 0)
        printf("Could not do the chain computation\n");

    printf("Contents after addition: \n");
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        printf("%.1f ", res[i]);
        if ((i + 1) % WIDTH == 0)
            printf("\n");
    }
    printf("\n");

    gpgpu_deinit();
    free(payload);
    free(a1);
    free(res);
    return 0;
}
