#include "computations.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "gpgpu_gles.h"

#define DEBUG 0

extern int HEIGHT;
extern int WIDTH;

void array_add_float()
{
    clock_t start;
    int gpu_time, cpu_time;
    // create two float arrays
    float* a1 = malloc(WIDTH * HEIGHT * sizeof(float));
    float* a2 = malloc(WIDTH * HEIGHT * sizeof(float));
    float* res = malloc(WIDTH * HEIGHT * sizeof(float));
    float* res2 = malloc(WIDTH * HEIGHT * sizeof(float));

    generate_data_f(a1, a2);

#if DEBUG
    printf("Data before computation: \n");
    print_data_f(a1);
#endif

    start = clock();
    if (gpgpu_arrayAddition(a1, a2, res) != 0)
        printf("Could not do the array addition\n");

    gpu_time = clock() - start;

#if DEBUG
    printf("Contents after GPU addition: \n");
    print_data_f(res);
#endif

    start = clock();
    cpu_compute_array_add_float(a1, a2, res2);
    cpu_time = clock() - start;

#if DEBUG
    printf("Contents after CPU addition: \n");
    print_data_f(res2);
#endif

    printf("GPU time %f CPU time %f\n", ((float)gpu_time / (float) CLOCKS_PER_SEC), ((float)cpu_time/ (float) CLOCKS_PER_SEC));

    gpgpu_deinit();
    free(a1);
    free(a2);
    free(res);
    free(res2);
}

void array_add_fixed16()
{
    clock_t start;
    int gpu_time, cpu_time;
    // create two uint16 arrays (twice the size because we use half the memory)
    uint16_t* a1 = malloc(2 * WIDTH * HEIGHT * sizeof(uint16_t));
    uint16_t* a2 = malloc(2 * WIDTH * HEIGHT * sizeof(uint16_t));
    uint16_t* res = malloc(2 * WIDTH * HEIGHT * sizeof(uint16_t));
    uint16_t* res2 = malloc(2 * WIDTH * HEIGHT * sizeof(uint16_t));

    generate_data_f16(a1, a2);

#if DEBUG
    printf("Data before computation: \n");
    print_data_f16(a1);
#endif

    start = clock();
    if (gpgpu_arrayAddition_fixed16(a1, a2, res, 5) != 0)
        printf("Could not do the array addition\n");

    gpu_time = clock() - start;

#if DEBUG
    printf("Contents after GPU addition: \n");
    print_data_f16(res);
#endif

    start = clock();
    cpu_compute_array_add_fixed16(a1, a2, res2);
    cpu_time = clock() - start;

#if DEBUG
    printf("Contents after CPU addition: \n");
    print_data_f16(res2);
#endif

    printf("GPU time %f CPU time %f\n", ((float)gpu_time / (float) CLOCKS_PER_SEC), ((float)cpu_time/ (float) CLOCKS_PER_SEC));

    gpgpu_deinit();
    free(a1);
    free(a2);
    free(res);
    free(res2);
}

void conv2d_float(int size)
{
    clock_t start;
    int gpu_time, cpu_time;
    // create two float arrays
    float* a1 = malloc(WIDTH * HEIGHT * sizeof(float));
    float* res = malloc(WIDTH * HEIGHT * sizeof(float));
    float* res2 = malloc(WIDTH * HEIGHT * sizeof(float));

    float* kernel;
    float kernel_3[9] = {
        1.0, 1.0, 1.0,
        1.0, 1.0, 1.0,
        1.0, 1.0, 1.0
    };
    float kernel_5[25] = {
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
    };

    switch (size)
    {
        case 3:
            kernel = kernel_3;
            break;
        case 5:
            kernel = kernel_5;
            break;
    }

    generate_data_conv2d_f(a1);

#if DEBUG
    printf("Data before computation: \n");
    print_data_f(a1);
#endif

    start = clock();
    if (gpgpu_firConvolution2D(a1, kernel, size, res) != 0)
        printf("Could not do the array addition\n");

    gpu_time = clock() - start;

#if DEBUG
    printf("Contents after GPU addition: \n");
    print_data_f(res);
#endif

    start = clock();
    cpu_compute_conv2d_float(a1, kernel, size, res2);
    cpu_time = clock() - start;

#if DEBUG
    printf("Contents after CPU addition: \n");
    print_data_f(res2);
#endif

    printf("GPU time %f CPU time %f\n", ((float)gpu_time / (float) CLOCKS_PER_SEC), ((float)cpu_time/ (float) CLOCKS_PER_SEC));

    gpgpu_deinit();
    free(a1);
    free(res);
    free(res2);
}

void chain_add_float(int rep)
{
    clock_t start;
    int gpu_time, cpu_time;

    // create two float arrays
    float* a1 = malloc(WIDTH * HEIGHT * sizeof(float));
    float* res = malloc(WIDTH * HEIGHT * sizeof(float));
    float* res2 = malloc(WIDTH * HEIGHT * sizeof(float));

    generate_data_f(a1, a1);

#if DEBUG
    printf("Data before computation: \n");
    print_data_f(a1);
#endif

    EOperation* ops = malloc(rep * sizeof(EOperation));
    UOperationPayloadFloat* payload = malloc(rep * sizeof(UOperationPayloadFloat));

    start = clock();
    for (int i = 0; i < rep; ++i)
    {
        ops[i] = ADD_SCALAR_FLOAT;
        payload[i].s = 2.0;
    }
    if (gpgpu_chain_apply_float(ops, payload, rep, a1, res) != 0)
        printf("Could not do the chain scalar addition\n");

    gpu_time = clock() - start;

#if DEBUG
    printf("Contents after GPU addition: \n");
    print_data_f(res);
#endif

    start = clock();
    cpu_compute_chain_add_float(rep, 2.0, a1, res2);
    cpu_time = clock() - start;

#if DEBUG
    printf("Contents after CPU addition: \n");
    print_data_f(res2);
#endif

    printf("GPU time %f CPU time %f\n", ((float)gpu_time / (float) CLOCKS_PER_SEC), ((float)cpu_time/ (float) CLOCKS_PER_SEC));

    gpgpu_deinit();
    free(a1);
    free(res);
    free(res2);
    free(ops);
    free(payload);
}

void chain_conv2d_float(int rep, int size)
{
    clock_t start;
    int gpu_time, cpu_time;
    // create two float arrays
    float* a1 = malloc(WIDTH * HEIGHT * sizeof(float));
    float* res = malloc(WIDTH * HEIGHT * sizeof(float));
    float* res2 = malloc(WIDTH * HEIGHT * sizeof(float));

    float* kernel;
    float kernel_3[9] = {
        1.0, 1.0, 1.0,
        1.0, 1.0, 1.0,
        1.0, 1.0, 1.0
    };
    float kernel_5[25] = {
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
    };

    switch (size)
    {
        case 3:
            kernel = kernel_3;
            break;
        case 5:
            kernel = kernel_5;
            break;
    }

    generate_data_conv2d_f(a1);

#if DEBUG
    printf("Data before computation: \n");
    print_data_f(a1);
#endif
    EOperation* ops = malloc(2 * rep * sizeof(EOperation));
    UOperationPayloadFloat* payload = malloc(2 * rep * sizeof(UOperationPayloadFloat));

    start = clock();
    for (int i = 0; i < rep; i += 2)
    {
        ops[i] = FIR_CONV2D_FLOAT;
        payload[i].arr = kernel;
        ops[i + 1] = FIR_CONV2D_FLOAT;
        payload[i + 1].n = size;
    }

    start = clock();
    if (gpgpu_chain_apply_float(ops, payload, rep, a1, res) != 0)
        printf("Could not do the chain conv2d\n");

    gpu_time = clock() - start;

#if DEBUG
    printf("Contents after GPU addition: \n");
    print_data_f(res);
#endif

    start = clock();
    cpu_compute_chain_conv2d_float(rep, a1, kernel, size, res2);
    cpu_time = clock() - start;

#if DEBUG
    printf("Contents after CPU addition: \n");
    print_data_f(res2);
#endif

    printf("GPU time %f CPU time %f\n", ((float)gpu_time / (float) CLOCKS_PER_SEC), ((float)cpu_time/ (float) CLOCKS_PER_SEC));

    gpgpu_deinit();
    free(a1);
    free(res);
    free(res2);
    free(ops);
    free(payload);
}

void cpu_compute_array_add_float(float* a1, float* a2, float* res)
{
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
        res[i] = a1[i] + a2[i];
}

void generate_data_f(float* in1, float* in2)
{
    float maxVal = 10.0;
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        float val = ((float)rand() / (float)(RAND_MAX)) * maxVal;
        in1[i] = in2[i] = val;
    }
}

void print_data_f(float* a)
{
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        printf("%.1f ", a[i]);
        if ((i + 1) % WIDTH == 0)
            printf("\n");
    }
    printf("\n");
}

void cpu_compute_array_add_fixed16(uint16_t* a1, uint16_t* a2, uint16_t* res)
{
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
        res[i] = a1[i] + a2[i];
}

void print_data_f16(uint16_t* a)
{
    for (int i = 0; i < 2 * WIDTH * HEIGHT; ++i)
    {
        printf("%d ", a[i]);
        if ((i + 1) % WIDTH == 0)
            printf("\n");
    }
    printf("\n");
}

void generate_data_f16(uint16_t* in1, uint16_t* in2)
{
    uint16_t maxVal = 10;
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        uint16_t val = (rand() / RAND_MAX) * maxVal;
        in1[i] = in2[i] = val;
    }
}

void generate_data_conv2d_f(float* in1)
{
    float maxVal = 10.0;
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        float val = ((float)rand() / (float)(RAND_MAX)) * maxVal;
        in1[i] = val;
    }
}

void cpu_compute_conv2d_float(float* a1, float* kernel, int size, float* res)
{
    float tmp = 0.0;
    float tmp_res = 0.0;
    for (int i = 0; i < WIDTH; ++i)
    {
        for (int j = 0; j < HEIGHT; ++j)
        {
            tmp = a1[i * WIDTH + j];
            for (int k = 0; k < size * size; ++k)
            {
                tmp_res += tmp * kernel[k];
            }
            res[i * WIDTH + j] = tmp_res;
        }
    }
}

void cpu_compute_chain_add_float(int rep, float scalar, float* a1, float* res)
{
    for (int n = 0; n < rep; ++n)
    {
        for (int i = 0; i < WIDTH * HEIGHT; ++i)
            res[i] = a1[i] = a1[i] + scalar;
    }
}

void cpu_compute_chain_conv2d_float(int rep, float* a1, float* kernel, int size, float* res)
{
    for (int n = 0; n < rep; ++n)
    {
        cpu_compute_conv2d_float(a1, kernel, size, res);
        for (int i = 0; i < WIDTH * HEIGHT; ++i)
            a1[i] = res[i]; // copy old output as new input
    }
}

void noop()
{
    clock_t start;
    int gpu_time, cpu_time;
    // create two float arrays
    float* a1 = malloc(WIDTH * HEIGHT * sizeof(float));
    float* res = malloc(WIDTH * HEIGHT * sizeof(float));
    float* res2 = malloc(WIDTH * HEIGHT * sizeof(float));

    generate_data_f(a1, a1);

#if DEBUG
    printf("Data before computation: \n");
    print_data_f(a1);
#endif

    start = clock();
    if (gpgpu_noop(a1, res) != 0)
        printf("Could not do the array addition\n");

    gpu_time = clock() - start;

#if DEBUG
    printf("Contents after GPU addition: \n");
    print_data_f(res);
#endif

    start = clock();
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
        res2[i] = a1[i];
    cpu_time = clock() - start;

#if DEBUG
    printf("Contents after CPU addition: \n");
    print_data_f(res2);
#endif

    printf("GPU time %f CPU time %f\n", ((float)gpu_time / (float) CLOCKS_PER_SEC), ((float)cpu_time/ (float) CLOCKS_PER_SEC));

    gpgpu_deinit();
    free(a1);
    free(res);
    free(res2);
}
