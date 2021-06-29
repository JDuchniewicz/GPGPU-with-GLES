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

int GPGPU_API gpgpu_init(uint32_t height, uint32_t width)
{
    int ret = 0;
    int major, minor;//, num_devices;
    // EGLDeviceEXT* devices = NULL;

    ///// load the extensions (hidden symbols???)
    //_eglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC) eglGetProcAddress("eglQueryDevicesEXT");
    //if (!_eglQueryDevicesEXT)
    //    ERR("Could not import eglQueryDevicesEXT()");

    //_eglQueryDeviceStringEXT = (PFNEGLQUERYDEVICESTRINGEXTPROC) eglGetProcAddress("eglQueryDeviceStringEXT");
    //if (!_eglQueryDeviceStringEXT)
    //    ERR("Could not import eglQueryDeviceStringEXT()");

    //_eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC) eglGetProcAddress("eglGetPlatformDisplayEXT");
    //if (!_eglGetPlatformDisplayEXT)
    //    ERR("Could not import eglGetPlatformDisplayEXT()");

    //// query the devices available (first call to allocate enough memory)
    //if (!_eglQueryDevicesEXT(0, NULL, &num_devices) || num_devices < 1)
    //    ERR("Not enough or no devices available");

    //devices = (EGLDeviceEXT*) malloc(sizeof(EGLDeviceEXT) * num_devices);
    //if (!devices)
    //    ERR("Could not allocate memory");

    //if (!_eglQueryDevicesEXT(num_devices, devices, &num_devices) || num_devices < 1)
    //    ERR("Could not get all devices after allocating");

    //// enumerate the devices
    //for (int i = 0; i < num_devices; ++i)
    //{
    //    const char* dev = _eglQueryDeviceStringEXT(devices[i], EGL_DRM_DEVICE_FILE_EXT);
    //    printf("Device 0x%.8lx: %s\n", (unsigned long)devices[i], dev ? dev : "NULL");
    //}

    // create the headless context
    //g_helper.display = _eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, devices[1], NULL);
    //printf("DISPLAY: %lu\n", (unsigned long)g_helper.display);
    //printf("EGL ERROR %d\n", eglGetError());
    //if (!g_helper.display)
    //    ERR("Could not get EXT display");

    //if (g_helper.display == EGL_NO_DISPLAY)
    //{
        //g_helper.display = eglGetDisplay(0);
    //}


    g_helper.gbd_fd = open("/dev/dri/renderD128", O_RDWR);
    if (g_helper.gbd_fd <= 0)
        ERR("Could not open device");

    g_helper.gbm = gbm_create_device(g_helper.gbd_fd);
    if (!g_helper.gbm)
        ERR("Could not create GBM device");

    g_helper.display = eglGetDisplay((EGLNativeDisplayType)g_helper.gbm);
    if (!g_helper.display)
        ERR("Could not create display");

    if (eglInitialize(g_helper.display, &major, &minor) == 0)
        ERR("Could not initialize display");

    printf("EGL API version %d.%d\n", major, minor);

    if (gpgpu_check_egl_extensions() != 0)
        ERR("Not enough extensions supported");


    if (eglBindAPI(EGL_OPENGL_ES_API) == 0)
        ERR("Could not bind the API");

    g_helper.gbm_surface = gbm_surface_create(g_helper.gbm, width, height, GBM_FORMAT_XRGB8888, GBM_BO_USE_RENDERING);
    if (!g_helper.gbm_surface)
        ERR("Could not create GBM surface");

    g_helper.surface = eglCreateWindowSurface(g_helper.display, g_helper.config, g_helper.gbm_surface, NULL);
    if (g_helper.surface == EGL_NO_SURFACE)
        ERR("Could not create EGL surface");

    return ret;

bail:
    // release all resources
    eglTerminate(g_helper.display);
    gbm_device_destroy(g_helper.gbm);
    close(g_helper.gbd_fd);

    return ret;
}

int GPGPU_API gpgpu_deinit()
{
    eglTerminate(g_helper.display);
    gbm_device_destroy(g_helper.gbm);
    close(g_helper.gbd_fd);
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

static int gpgpu_check_egl_extensions()
{
    int ret = 0;
    if (!g_helper.display)
        ERR("No display created!");
    const char* egl_extensions = eglQueryString(g_helper.display, EGL_EXTENSIONS);
    if (!strstr(egl_extensions, "EGL_KHR_create_context"))
        ERR("No EGL_KHR_create_context extension");
    if (!strstr(egl_extensions, "EGL_KHR_surfaceless_context"))
        ERR("No EGL_KHR_create_context extension");

bail:
    return ret;
}

static int gpgpu_find_matching_config(EGLConfig* config, uint32_t gbm_format)
{
    int ret = 0;
    if (!g_helper.display)
        ERR("No display created!");

    EGLint count;
    static const EGLint config_attrs[] = {
        EGL_BUFFER_SIZE,        32,
        EGL_DEPTH_SIZE,         EGL_DONT_CARE,
        EGL_STENCIL_SIZE,       EGL_DONT_CARE,
        EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE,       EGL_WINDOW_BIT,
        EGL_NONE
    };
    if (!eglGetConfigs(g_helper.display, NULL, 0, &count))
        ERR("Could not get number of configs.");
//    if (!eglChooseConfig(g_helper.display, , EGLConfig *configs, EGLint config_size, EGLint *num_config)
//    EGLConfig* configs =


    if (eglChooseConfig(g_helper.display, config_attrs, &g_helper.config, 1, &count) == 0)
        ERR("Could not create config");
bail:
    return ret;
}
