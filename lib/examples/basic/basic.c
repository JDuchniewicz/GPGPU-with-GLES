#include <stdio.h>
#include "gpgpu_gles.h"

int main()
{
    if (gpgpu_init() != 0)
    {
        printf("Could not initialize the API\n");
        return 0;
    }

    // create two int arrays
    int* a1 = malloc(WIDTH * HEIGHT * sizeof(int));
    int* a2 = malloc(WIDTH * HEIGHT * sizeof(int));
    int* res = malloc(WIDTH * HEIGHT * sizeof(int));

    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        a1[i] = a2[i] = i;
    }

    if (gpgpu_arrayAddition(a1, a2, WIDTH * HEIGHT, res) != 0)
        printf("Could not do the array addition\n");

    printf("Contents after addition: \n");
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        printf("%d ", res[i]);
        if (i % WIDTH == 0)
            printf("\n");
    }
    printf("\n");

    gpgpu_deinit();
    free(a1);
    free(a2);
    free(res);
    return 0;
}
