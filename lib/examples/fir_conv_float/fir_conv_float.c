#include <stdio.h>
#include "gpgpu_gles.h"

#define HEIGHT 8
#define WIDTH HEIGHT

int main()
{
    if (gpgpu_init(HEIGHT, WIDTH) != 0)
    {
        printf("Could not initialize the API\n");
        return 0;
    }

    // create two float arrays
    float* a1 = malloc(WIDTH * HEIGHT * sizeof(float));
    float* res = malloc(WIDTH * HEIGHT * sizeof(float));

    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        a1[i] = i;
    }

    //float kernel[9] = {
    //    1.0, 1.0, 1.0,
    //    1.0, 1.0, 1.0,
    //    1.0, 1.0, 1.0
    //};
    //float kernel[9] = {
    //    0.0, 0.0, 0.0,
    //    0.0, 1.0, 0.0,
    //    0.0, 0.0, 0.0
    //};

    float kernel[25] = {
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
    };

    printf("Data before computation: \n");
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        printf("%.1f ", a1[i]);
        if ((i + 1) % WIDTH == 0)
            printf("\n");
    }
    printf("\n");

    if (gpgpu_firConvolution2D(a1, kernel, 5, res) != 0)
        printf("Could not do the FIR filtering\n");

    printf("Contents after FIR filtering: \n");
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        printf("%.1f ", res[i]);
        if ((i + 1) % WIDTH == 0)
            printf("\n");
    }
    printf("\n");

    gpgpu_deinit();
    free(a1);
    free(res);
    return 0;
}
