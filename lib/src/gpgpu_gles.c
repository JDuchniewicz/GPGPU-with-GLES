#include "gpgpu_gles.h"
#include "gpgpu_int.h"
#include "debug.h"
#include "shaders.h"

GLHelper g_helper;

/////////////////////////
////* API FUNCTIONS *////
/////////////////////////

int GPGPU_API gpgpu_init(uint32_t height, uint32_t width)
{
    int ret = 0;
    int major, minor;

#ifndef BEAGLE
    g_helper.gbd_fd = open("/dev/dri/renderD128", O_RDWR);
    if (g_helper.gbd_fd <= 0)
        ERR("Could not open device");

    g_helper.gbm = gbm_create_device(g_helper.gbd_fd);
    if (!g_helper.gbm)
        ERR("Could not create GBM device");

    g_helper.display = eglGetDisplay((EGLNativeDisplayType)g_helper.gbm);
    if (!g_helper.display)
        ERR("Could not create display");
#else
    g_helper.display = eglGetDisplay((EGLNativeDisplayType)0);
    if (!g_helper.display)
        ERR("Could not create display");
#endif

    if (eglInitialize(g_helper.display, &major, &minor) == 0)
        ERR("Could not initialize display");

    printf("EGL API version %d.%d\n", major, minor);

    if (gpgpu_check_egl_extensions() != 0)
        ERR("Not enough extensions supported");

#ifndef BEAGLE
    if (gpgpu_find_matching_config(&g_helper.config, GBM_FORMAT_ARGB8888) != 0) // 9 is RGBA8888 on BBB
        ERR("Could not find matching config");
#else
    if (gpgpu_find_matching_config(&g_helper.config, 9) != 0) // 9 is RGBA8888 on BBB
        ERR("Could not find matching config");
#endif

    if (eglBindAPI(EGL_OPENGL_ES_API) == 0)
        ERR("Could not bind the API");

#ifndef BEAGLE
    g_helper.gbm_surface = gbm_surface_create(g_helper.gbm, width, height, GBM_FORMAT_ARGB8888, GBM_BO_USE_RENDERING);
    if (!g_helper.gbm_surface)
        ERR("Could not create GBM surface");
    g_helper.surface = eglCreateWindowSurface(g_helper.display, g_helper.config, g_helper.gbm_surface, NULL);
#else
    g_helper.surface = eglCreateWindowSurface(g_helper.display, g_helper.config, (EGLNativeWindowType)0, NULL); //g_helper.gbm_surface, NULL);
#endif

    if (g_helper.surface == EGL_NO_SURFACE)
        ERR("Could not create EGL surface");

    EGLint eglContextAttributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    g_helper.context = eglCreateContext(g_helper.display, g_helper.config, EGL_NO_CONTEXT, eglContextAttributes);
    if (g_helper.context == EGL_NO_CONTEXT)
        ERR("Could not create EGL context");

    if (eglMakeCurrent(g_helper.display, g_helper.surface, g_helper.surface, g_helper.context) != EGL_TRUE) // EGL_NO_SURFACE??
        ERR("Could not bind the surface to context");

    EGLint version = 0;
    eglQueryContext(g_helper.display, g_helper.context, EGL_CONTEXT_CLIENT_VERSION, &version);
    printf("EGL Context version: %d\n", version);

    if (gpgpu_make_FBO(WIDTH, HEIGHT) != 0)
        ERR("Could not create FBO");

    return ret;
bail:
    // release all resources
    gpgpu_deinit();

    return ret;
}

// for now this api is single-shot only (textures could be reused between calls)
int GPGPU_API gpgpu_deinit()
{
    eglDestroySurface(g_helper.display, g_helper.surface);
    eglDestroyContext(g_helper.display, g_helper.context);
    eglTerminate(g_helper.display);
#ifndef BEAGLE
    gbm_device_destroy(g_helper.gbm);
    close(g_helper.gbd_fd);
#endif
    return 0;
}

int GPGPU_API gpgpu_arrayAddition(float* a1, float* a2, float* res)
{
    int ret = 0;
    unsigned char* buffer = malloc(4 * WIDTH * HEIGHT);
    GLuint texId0, texId1;
    gpgpu_make_texture(a1, WIDTH, HEIGHT, &texId0);
    gpgpu_make_texture(a2, WIDTH, HEIGHT, &texId1);

#if DEBUG
    printf("RAW contents before addition: \n");
    for (int i = 0; i < 4 * WIDTH * HEIGHT; ++i)
    {
        printf("%d ", *((unsigned char*)a1 + i));
        if ((i + 1)  % (4 * WIDTH) == 0)
            printf("\n");
    }
    printf("\n");
#endif

    // inputs are float textures, output is a vec4 of unsigned bytes representing the float result of one texel
    // we need to extract the bits following the IEEE754 floating point format because GLES 2.0 does not have bit extraction
    gpgpu_build_program(REGULAR, ARRAY_ADD_FLOAT);

    // create the geometry to draw the texture on
    GLuint geometry;
    glGenBuffers(1, &geometry);
    glBindBuffer(GL_ARRAY_BUFFER, geometry);
    glBufferData(GL_ARRAY_BUFFER, 20*sizeof(float), gpgpu_geometry, GL_STATIC_DRAW);

    // setup the vertex position as the attribute of vertex shader
    gpgpu_add_attribute("position", 3, 20, 0);
    gpgpu_add_attribute("texCoord", 2, 20, 3);
    // do the actual computation
    // bind textures to their respective texturing units
    // add texture uniforms to fragment shader
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texId0);
    gpgpu_add_uniform("texture0", 0, "uniform1i");

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, texId1);
    gpgpu_add_uniform("texture1", 1, "uniform1i");

    glActiveTexture(GL_TEXTURE0);

    if (gpgpu_report_glError(glGetError()) != 0)
        ERR("Could not prepare textures");

    // finally draw it
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //////////
    // magic happens and the data is now ready
    // poof!
    //////////

    glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    // convert from unsigned bytes back to the original format (float?)

#if DEBUG
    printf("RAW contents after addition: \n");
    for (int i = 0; i < 4 * WIDTH * HEIGHT; ++i)
    {
        printf("%d ", buffer[i]);
        if ((i + 1)  % (4 * WIDTH) == 0)
            printf("\n");
    }
    printf("\n");
#endif

    // copy the bytes as floats TODO: remove this copy and instead reinterpret the bytes
    for (int i = 0; i < 4 * WIDTH * HEIGHT; i += 4)
    {
        res[i / 4] = *((float*)buffer + i / 4);
    }

bail:
    // TODO: what should be released upon failure?
    free(buffer);
    return ret;
}

// TODO: need figuring out a good width and height specification
int GPGPU_API gpgpu_firConvolution2D(float* data, float* kernel, int size, float* res)
{
    // if width != height abort? TODO:
    int ret = 0;
    unsigned char* buffer = malloc(4 * WIDTH * HEIGHT);
    GLuint texId0, texId1;
    gpgpu_make_texture(data, WIDTH, HEIGHT, &texId0);
    gpgpu_make_texture(kernel, size, size, &texId1);

#if DEBUG
    printf("RAW contents before addition: \n");
    for (int i = 0; i < 4 * WIDTH * HEIGHT; ++i)
    {
        printf("%d ", *((unsigned char*)data + i));
        if ((i + 1)  % (4 * WIDTH) == 0)
            printf("\n");
    }
    printf("\n");
#endif

    gpgpu_build_program(REGULAR, FIR_CONV_FLOAT);

    // create the geometry to draw the texture on
    GLuint geometry;
    glGenBuffers(1, &geometry);
    glBindBuffer(GL_ARRAY_BUFFER, geometry);
    glBufferData(GL_ARRAY_BUFFER, 20*sizeof(float), gpgpu_geometry, GL_STATIC_DRAW);

    // setup the vertex position as the attribute of vertex shader
    gpgpu_add_attribute("position", 3, 20, 0);
    gpgpu_add_attribute("texCoord", 2, 20, 3);
    // do the actual computation
    // bind textures to their respective texturing units
    // add texture uniforms to fragment shader
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texId0);
    gpgpu_add_uniform("texture0", 0, "uniform1i");

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, texId1);
    gpgpu_add_uniform("texture1", 1, "uniform1i");

    // add the texture step =  1 / width
    gpgpu_add_uniform("w", 1.0 / WIDTH, "uniform1f");

    glActiveTexture(GL_TEXTURE0);

    if (gpgpu_report_glError(glGetError()) != 0)
        ERR("Could not prepare textures");

    // finally draw it
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //////////
    // magic happens and the data is now ready
    // poof!
    //////////

    glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    // convert from unsigned bytes back to the original format (float?)

#if DEBUG
    printf("RAW contents after addition: \n");
    for (int i = 0; i < 4 * WIDTH * HEIGHT; ++i)
    {
        printf("%d ", buffer[i]);
        if ((i + 1)  % (4 * WIDTH) == 0)
            printf("\n");
    }
    printf("\n");
#endif

    // copy the bytes as floats TODO: remove this copy and instead reinterpret the bytes
    for (int i = 0; i < 4 * WIDTH * HEIGHT; i += 4)
    {
        res[i / 4] = *((float*)buffer + i / 4);
    }

bail:
    free(buffer);
    return ret;
}

int GPGPU_API gpgpu_matrixMultiplication(int* a, int* b, int size, int* res)
{

    return 0;
}

int GPGPU_API gpgpu_arrayAddition_fixed16(uint16_t* a1, uint16_t* a2, uint16_t* res, uint8_t fractional_bits)
{
    int ret = 0;
    int fraction_divider = 1 << fractional_bits;
    unsigned char* buffer = malloc(4 * WIDTH * HEIGHT);
    GLuint texId0, texId1;
    gpgpu_make_texture(a1, WIDTH, HEIGHT, &texId0);
    gpgpu_make_texture(a2, WIDTH, HEIGHT, &texId1);

#if DEBUG
    printf("RAW contents before addition: \n");
    for (int i = 0; i < 4 * WIDTH * HEIGHT; ++i)
    {
        printf("%d ", *((unsigned char*)a1 + i));
        if ((i + 1)  % (4 * WIDTH) == 0)
            printf("\n");
    }
    printf("\n");
#endif

    gpgpu_build_program(REGULAR, ARRAY_ADD_FIXED16);

    // create the geometry to draw the texture on
    GLuint geometry;
    glGenBuffers(1, &geometry);
    glBindBuffer(GL_ARRAY_BUFFER, geometry);
    glBufferData(GL_ARRAY_BUFFER, 20*sizeof(float), gpgpu_geometry, GL_STATIC_DRAW);

    // setup the vertex position as the attribute of vertex shader
    gpgpu_add_attribute("position", 3, 20, 0);
    gpgpu_add_attribute("texCoord", 2, 20, 3);
    // do the actual computation
    // bind textures to their respective texturing units
    // add texture uniforms to fragment shader
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texId0);
    gpgpu_add_uniform("texture0", 0, "uniform1i");

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, texId1);
    gpgpu_add_uniform("texture1", 1, "uniform1i");

    gpgpu_add_uniform("fraction_divider", fraction_divider, "uniform1i");

    glActiveTexture(GL_TEXTURE0);

    if (gpgpu_report_glError(glGetError()) != 0)
        ERR("Could not prepare textures");

    // finally draw it
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //////////
    // magic happens and the data is now ready
    // poof!
    //////////

    glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    // convert from unsigned bytes back to the original format

#if DEBUG
    printf("RAW contents after addition: \n");
    for (int i = 0; i < 4 * WIDTH * HEIGHT; ++i)
    {
        printf("%d ", buffer[i]);
        if ((i + 1)  % (4 * WIDTH) == 0)
            printf("\n");
    }
    printf("\n");
#endif

    // copy the bytes as floats TODO: remove this copy and instead reinterpret the bytes
    for (int i = 0; i < 4 * WIDTH * HEIGHT; i += 2)
    {
        res[i / 2] = *((uint16_t*)buffer + i / 2);
    }

bail:
    free(buffer);
    return ret;

}
