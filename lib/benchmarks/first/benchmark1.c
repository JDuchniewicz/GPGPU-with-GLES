#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "gpgpu_gles.h"

//#define HEIGHT 4096
//#define WIDTH HEIGHT
#define DEBUG 0

// TODO: add time measurements

void cpu_compute(float* a1, float* a2, float* res);
void generate_data(float* in1, float* in2);
void parse_args(int argc, char** argv);
void print_data_f(float* a);
void print_benchmark_names();
int size_valid();
int name_valid();

typedef enum
{
    NONE = -1,
    ARRAY_ADD_FLOAT,
    ARRAY_ADD_FIXED16,
} EBenchmarkType;

typedef struct
{
    char* name;
    EBenchmarkType type;
} benchmark_t;

benchmark_t benchmark_types[] = {
    { .name = "array_add_float", .type = ARRAY_ADD_FLOAT }, { .name = "array_add_fixed16", .type = ARRAY_ADD_FIXED16 },
};
int benchmark_length = 2;

int HEIGHT, WIDTH;
char* NAME;
EBenchmarkType TYPE;

// first argument is the size of array/matrix used
// second argument is the type of computation? TODO: add some args parsing?
int main(int argc, char** argv)
{
    clock_t start;
    int gpu_time, cpu_time;
    srand((unsigned int)time(NULL));

    parse_args(argc, argv);

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
    cpu_compute(a1, a2, res2);
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

void parse_args(int argc, char** argv)
{
    int opt;

    while ((opt = getopt(argc, argv, "s:n:")) != -1)
    {
        switch (opt)
        {
            case 's':
                WIDTH = strtol(optarg, NULL, 10);
                HEIGHT = WIDTH;
                if (size_valid() != 0)
                {
                    fprintf(stderr, "-s (Size) must be > 0, power of 2 and not greater than 2048\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'n':
                NAME = optarg;
                if (name_valid() != 0)
                {
                    fprintf(stderr, "Invalid benchmark name\n");
                    print_benchmark_names();
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                fprintf(stderr, "Usage: %s [-s] [size] [-n] [benchmark_name]\n", argv[0]);
                print_benchmark_names();
                exit(EXIT_FAILURE);
        }
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

int size_valid()
{
    int ret = 0;
    if ((WIDTH & (WIDTH - 1)))
        ret = -1;
    if (WIDTH == 0)
        ret = -1;
    if (WIDTH > 2048) // TODO: query size dynamically?
        ret = -1;

    return ret;
}

int name_valid()
{
    int ret = -1;

    for (int i = 0; i < benchmark_length; ++i)
    {
        if (strcmp(NAME, benchmark_types[i].name) == 0)
        {
            ret = 0;
            TYPE = benchmark_types[i].type;
            break;
        }
    }
    return ret;
}

void print_benchmark_names()
{
    fprintf(stderr, "Possible benchmark names: \n");
    for (int i = 0; i < benchmark_length; ++i)
    {
        fprintf(stderr, "%s\t", benchmark_types[i].name);
        if (i % 2 == 0)
            fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}
