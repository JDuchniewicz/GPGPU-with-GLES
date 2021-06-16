#include "gpgpu_gles.h"

GLHelper g_helper;

int GPGPU_API gpgpu_init()
{
    int ret = 0;
    int major, minor;
    // create the headless context
    g_helper.display = eglGetDisplay(0); // apparently this does not work with headless

    if (g_helper.display == EGL_NO_DISPLAY)
    {
        g_helper.display = eglGetDisplay((EGLNativeDisplayType)EGL_DEFAULT_DISPLAY);
    }

    if (eglInitialize(g_helper.display, &major, &minor) == 0)
    {
        return -1;
    }

    if (eglBindAPI(EGL_OPENGL_ES_API) == 0)
    {
        return -1;
    }
    return 0;
}

int GPGPU_API gpgpu_deinit()
{

    return 0;
}

int GPGPU_API gpgpu_arrayAddition(int* a1, int* a2, int len, int* res)
{

    return 0;
}

int GPGPU_API gpgpu_firConvolution(int* data, int len, int* kernel, int size, int* res)
{

    return 0;
}

int GPGPU_API gpgpu_matrixMultiplication(int* a, int* b, int size, int* res)
{

    return 0;
}
