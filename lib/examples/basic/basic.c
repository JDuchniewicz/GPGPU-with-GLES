#include <stdio.h>
#include "gpgpu_gles.h"

int main()
{
    if (gpgpu_init() != 0)
    {
        printf("Could not initialize the API\n");
        return 0;
    }

    // create two float arrays
    float a1[10] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    float a2[10] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    float* res = malloc(10 * sizeof(float));

    if (gpgpu_arrayAddition(a1, a2, 1024, res) != 0)
        printf("Could not do the array addition\n");

    printf("Contents after addition: \n");
    for (int i = 0; i < 10; ++i)
        printf("%f ", res[i]);
    printf("\n");

    gpgpu_deinit();
    free(res);
    return 0;
}
