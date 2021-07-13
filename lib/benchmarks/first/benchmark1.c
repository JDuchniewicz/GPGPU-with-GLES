#include <stdio.h>
#include <time.h>
#include "gpgpu_gles.h"

#define HEIGHT 4096
#define WIDTH HEIGHT
#define DEBUG 0

// TODO: add time measurements

void cpu_compute(float* a1, float* a2, float* res);
void generate_data(float* in1, float* in2);

// first argument is the size of array/matrix used
// second argument is the type of computation? TODO: add some args parsing?
int main()
{
    clock_t start;
    int gpu_time, cpu_time;
    srand((unsigned int)time(NULL));

    if (gpgpu_init(HEIGHT, WIDTH) != 0)
    {
        printf("Could not initialize the API\n");
        return 0;
    }

    // create two float arrays
    float* a1 = malloc(WIDTH * HEIGHT * sizeof(float));
    float* a2 = malloc(WIDTH * HEIGHT * sizeof(float));
    float* res = malloc(WIDTH * HEIGHT * sizeof(float));
    float* res2 = malloc(WIDTH * HEIGHT * sizeof(float));

    generate_data(a1, a2);

#if DEBUG
    printf("Data before computation: \n");
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        printf("%.1f ", a1[i]);
        if ((i + 1) % WIDTH == 0)
            printf("\n");
    }
    printf("\n");
#endif

    start = clock();
    if (gpgpu_arrayAddition(a1, a2, res) != 0)
        printf("Could not do the array addition\n");

    gpu_time = clock() - start;

#if DEBUG
    printf("Contents after GPU addition: \n");
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        printf("%.1f ", res[i]);
        if ((i + 1) % WIDTH == 0)
            printf("\n");
    }
    printf("\n");
#endif

    start = clock();
    cpu_compute(a1, a2, res2);
    cpu_time = clock() - start;

#if DEBUG
    printf("Contents after CPU addition: \n");
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        printf("%.1f ", res2[i]);
        if ((i + 1) % WIDTH == 0)
            printf("\n");
    }
    printf("\n");
#endif

    printf("GPU time %f CPU time %f\n", ((float)gpu_time / (float) CLOCKS_PER_SEC), ((float)cpu_time/ (float) CLOCKS_PER_SEC));

    gpgpu_deinit();
    free(a1);
    free(a2);
    free(res);
    free(res2);
    return 0;
}

void generate_data(float* in1, float* in2)
{
    float maxFloat = 10.0;
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        float val = ((float)rand() / (float)(RAND_MAX)) * maxFloat;
        in1[i] = in2[i] = val;
    }
}

void cpu_compute(float* a1, float* a2, float* res)
{
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
        res[i] = a1[i] + a2[i];
}
