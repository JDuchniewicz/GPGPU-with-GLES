#include "gpgpu_gles.h"

GLHelper g_helper;
PFNEGLQUERYDEVICESEXTPROC _eglQueryDevicesEXT = NULL;
PFNEGLQUERYDEVICESTRINGEXTPROC _eglQueryDeviceStringEXT = NULL;
PFNEGLGETPLATFORMDISPLAYEXTPROC _eglGetPlatformDisplayEXT = NULL;

#define ERR(m) { \
    ret = -1; \
    fprintf(stderr, "ERROR in line %d: %s\n", __LINE__, m); \
    goto bail; \
}

// TODO: map GL errors?

int GPGPU_API gpgpu_init()
{
    int ret = 0;
    int major, minor, num_devices;
    EGLDeviceEXT* devices = NULL;

    /// load the extensions (hidden symbols???)
    _eglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC) eglGetProcAddress("eglQueryDevicesEXT");
    if (!_eglQueryDevicesEXT)
        ERR("Could not import eglQueryDevicesEXT()");

    _eglQueryDeviceStringEXT = (PFNEGLQUERYDEVICESTRINGEXTPROC) eglGetProcAddress("eglQueryDeviceStringEXT");
    if (!_eglQueryDeviceStringEXT)
        ERR("Could not import eglQueryDeviceStringEXT()");

    _eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC) eglGetProcAddress("eglGetPlatformDisplayEXT");
    if (!_eglGetPlatformDisplayEXT)
        ERR("Could not import eglGetPlatformDisplayEXT()");

    // query the devices available (first call to allocate enough memory)
    if (!_eglQueryDevicesEXT(0, NULL, &num_devices) || num_devices < 1)
        ERR("Not enough or no devices available");

    devices = (EGLDeviceEXT*) malloc(sizeof(EGLDeviceEXT) * num_devices);
    if (!devices)
        ERR("Could not allocate memory");

    if (!_eglQueryDevicesEXT(num_devices, devices, &num_devices) || num_devices < 1)
        ERR("Could not get all devices after allocating");

    // enumerate the devices
    for (int i = 0; i < num_devices; ++i)
    {
        const char* dev = _eglQueryDeviceStringEXT(devices[i], EGL_DRM_DEVICE_FILE_EXT);
        printf("Device 0x%.8lx: %s\n", (unsigned long)devices[i], dev ? dev : "NULL");
    }

    // create the headless context
    g_helper.display = _eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, devices[0], NULL);
    if (!devices)
        ERR("Could not get EXT display");
    g_helper.display = eglGetDisplay(0); // apparently this does not work with headless

    if (g_helper.display == EGL_NO_DISPLAY)
    {
        g_helper.display = eglGetDisplay((EGLNativeDisplayType)EGL_DEFAULT_DISPLAY);
    }

    if (eglInitialize(g_helper.display, &major, &minor) == 0)
        ERR("Could not initialize display");

    if (eglBindAPI(EGL_OPENGL_ES_API) == 0)
        ERR("Could not bind the API");

    return ret;

bail:
    // release all resources
    return ret;
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
