#include <stdio.h>
#include "gpgpu_gles.h"

int main()
{
    if (gpgpu_init() != 0)
    {
        printf("Could not initialize the API\n");
        return 0;
    }

    // use just 16 bits of the buffer now
    uint16_t* a1 = malloc(WIDTH * HEIGHT * sizeof(uint16_t));
    uint16_t* a2 = malloc(WIDTH * HEIGHT * sizeof(uint16_t));
    uint16_t* res = malloc(WIDTH * HEIGHT * sizeof(uint16_t));

    // intialization of dummy integer values
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        a1[i] = a2[i] = i;
    }

    printf("Data before computation: \n");
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        printf("%d ", a1[i]);
        if ((i + 1) % WIDTH == 0)
            printf("\n");
    }
    printf("\n");

    if (gpgpu_arrayAddition_fixed16_rgb565(a1, a2, res, 5) != 0)
        printf("Could not do the array addition\n");

    printf("Contents after addition: \n");
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
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
