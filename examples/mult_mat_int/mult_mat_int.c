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
    int* a = malloc(WIDTH * HEIGHT * sizeof(float));
    int* b = malloc(WIDTH * HEIGHT * sizeof(float));
    int* res = malloc(WIDTH * HEIGHT * sizeof(float));

    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        a[i] = b[i] = i;
    }

    printf("Data before computation: \n");
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        printf("%d ", a[i]);
        if ((i + 1) % WIDTH == 0)
            printf("\n");
    }
    printf("\n");

    if (gpgpu_matrixMultiplication(a, b, 4, res) != 0)
        printf("Could not do the matrix multiplication\n");

    printf("Contents after multiplication: \n");
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        printf("%d ", res[i]);
        if ((i + 1) % WIDTH == 0)
            printf("\n");
    }
    printf("\n");

    gpgpu_deinit();
    free(a);
    free(b);
    free(res);
    return 0;
}