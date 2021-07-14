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

    // create two float arrays
    float* a1 = malloc(WIDTH * HEIGHT * sizeof(float));
    float* a2 = malloc(WIDTH * HEIGHT * sizeof(float));
    float* res = malloc(WIDTH * HEIGHT * sizeof(float));

    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        a1[i] = a2[i] = i;
    }

    printf("Data before computation: \n");
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        printf("%.1f ", a1[i]);
        if ((i + 1) % WIDTH == 0)
            printf("\n");
    }
    printf("\n");

    if (gpgpu_arrayAddition(a1, a2, res) != 0)
        printf("Could not do the array addition\n");

    printf("Contents after addition: \n");
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        printf("%.1f ", res[i]);
        if ((i + 1) % WIDTH == 0)
            printf("\n");
    }
    printf("\n");

    gpgpu_deinit();
    free(a1);
    free(a2);
    free(res);
    return 0;
}
