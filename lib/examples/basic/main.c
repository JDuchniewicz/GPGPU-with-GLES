#include <stdio.h>
#include "../../include/gpgpu_gles.h" // TODO: change

int main()
{
    if (gpgpu_init() != 0)
    {
        printf("Could not initialize the API\n");
    }
    return 0;
}
