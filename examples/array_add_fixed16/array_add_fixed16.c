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

    // create two uint16 arrays (twice the size because we use half the memory)
    uint16_t* a1 = malloc(2 * WIDTH * HEIGHT * sizeof(uint16_t));
    uint16_t* a2 = malloc(2 * WIDTH * HEIGHT * sizeof(uint16_t));
    uint16_t* res = malloc(2 * WIDTH * HEIGHT * sizeof(uint16_t));

    // intialization of dummy integer values
    for (int i = 0; i < 2 * WIDTH * HEIGHT; ++i)
    {
        a1[i] = a2[i] = i;
    }

    printf("Data before computation: \n");
    for (int i = 0; i < 2 * WIDTH * HEIGHT; ++i)
    {
        printf("%d ", a1[i]);
        if ((i + 1) % WIDTH == 0)
            printf("\n");
    }
    printf("\n");

    if (gpgpu_arrayAddition_fixed16(a1, a2, res, 5) != 0)
        printf("Could not do the array addition\n");

    printf("Contents after addition: \n");
    for (int i = 0; i < 2 * WIDTH * HEIGHT; ++i)
    {
        printf("%d ", res[i]);
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
