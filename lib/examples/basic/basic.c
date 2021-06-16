#include <stdio.h>
#include "gpgpu_gles.h"

int main()
{
    if (gpgpu_init() != 0)
    {
        printf("Could not initialize the API\n");
    }
    gpgpu_deinit();
    return 0;
}
