#include "gpgpu_gles.h"
#include "gpgpu_int.h"
#include "debug.h"
#include "shaders.h"

GLHelper g_helper;
GChainHelper g_chainHelper;

/////////////////////////
////* API FUNCTIONS *////
/////////////////////////

int GPGPU_API gpgpu_init(int height, int width)
{
    int ret = 0;
    if (g_helper.state != INIT)
        ERR("Can call init only once!");

    int major, minor;

    g_helper.height = height;
    g_helper.width = width;

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
    EGLint eglSurfaceAttibutes[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_NONE
    };
    g_helper.surface = eglCreatePbufferSurface(g_helper.display, g_helper.config, eglSurfaceAttibutes);
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

    if (gpgpu_make_FBO() != 0)
        ERR("Could not create FBO");

    g_helper.state = READY;
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
    if (g_helper.state != READY) // TODO: probably no need to set the states in single-shot API
        ERR("Call gpgpu_init() first!");

    unsigned char* buffer = malloc(4 * g_helper.width * g_helper.height);
    GLuint texId0, texId1;
    gpgpu_make_texture(a1, g_helper.width, g_helper.height, &texId0);
    gpgpu_make_texture(a2, g_helper.width, g_helper.height, &texId1);

#if DEBUG
    printf("RAW contents before addition: \n");
    for (int i = 0; i < 4 * g_helper.width * g_helper.height; ++i)
    {
        printf("%d ", *((unsigned char*)a1 + i));
        if ((i + 1)  % (4 * g_helper.width) == 0)
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

    glReadPixels(0, 0, g_helper.width, g_helper.height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    // convert from unsigned bytes back to the original format (float?)

#if DEBUG
    printf("RAW contents after addition: \n");
    for (int i = 0; i < 4 * g_helper.width * g_helper.height; ++i)
    {
        printf("%d ", buffer[i]);
        if ((i + 1)  % (4 * g_helper.width) == 0)
            printf("\n");
    }
    printf("\n");
#endif

    // copy the bytes as floats
    for (int i = 0; i < 4 * g_helper.width * g_helper.height; i += 4)
    {
        res[i / 4] = *((float*)buffer + i / 4);
    }

bail:
    // TODO: what should be released upon failure?
    if (buffer)
        free(buffer);
    return ret;
}

// TODO: need figuring out a good width and height specification
int GPGPU_API gpgpu_firConvolution2D(float* data, float* kernel, int size, float* res)
{
    // if width != height abort? TODO:
    int ret = 0;
    if (g_helper.state != READY)
        ERR("Call gpgpu_init() first!");

    unsigned char* buffer = malloc(4 * g_helper.width * g_helper.height);
    GLuint texId0, texId1;
    gpgpu_make_texture(data, g_helper.width, g_helper.height, &texId0);
    gpgpu_make_texture(kernel, size, size, &texId1);

#if DEBUG
    printf("RAW contents before addition: \n");
    for (int i = 0; i < 4 * g_helper.width * g_helper.height; ++i)
    {
        printf("%d ", *((unsigned char*)data + i));
        if ((i + 1)  % (4 * g_helper.width) == 0)
            printf("\n");
    }
    printf("\n");
#endif

#ifndef BEAGLE
    switch (size)
    {
        case 3:
            gpgpu_build_program(REGULAR, FIR_CONV2D_FLOAT_3);
            break;
        case 5:
            if (g_helper.width < 5 || g_helper.height < 5)
                ERR("WIDTH && HEIGHT must be greater than 5");
            gpgpu_build_program(REGULAR, FIR_CONV2D_FLOAT_5);
            break;
        default:
            ERR("Supported kernel size: 3, 5");
    }
#else
    switch (size)
    {
        case 3:
            gpgpu_build_program(REGULAR, FIR_CONV2D_FLOAT_BBB_3);
            break;
        case 5:
            if (g_helper.width < 5 || g_helper.height < 5)
                ERR("WIDTH && HEIGHT must be greater than 5");
            gpgpu_build_program(REGULAR, FIR_CONV2D_FLOAT_BBB_5);
            break;
        default:
            ERR("Supported kernel size: 3, 5");
    }
#endif

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
    gpgpu_add_uniform("w", 1.0 / g_helper.width, "uniform1f");

    glActiveTexture(GL_TEXTURE0);

    if (gpgpu_report_glError(glGetError()) != 0)
        ERR("Could not prepare textures");

    // finally draw it
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //////////
    // magic happens and the data is now ready
    // poof!
    //////////

    glReadPixels(0, 0, g_helper.width, g_helper.height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    // convert from unsigned bytes back to the original format (float?)

#if DEBUG
    printf("RAW contents after addition: \n");
    for (int i = 0; i < 4 * g_helper.width * g_helper.height; ++i)
    {
        printf("%d ", buffer[i]);
        if ((i + 1)  % (4 * g_helper.width) == 0)
            printf("\n");
    }
    printf("\n");
#endif

    // copy the bytes as floats
    for (int i = 0; i < 4 * g_helper.width * g_helper.height; i += 4)
    {
        res[i / 4] = *((float*)buffer + i / 4);
    }

bail:
    if (buffer)
        free(buffer);
    return ret;
}

int GPGPU_API gpgpu_matrixMultiplication(int* a, int* b, int size, int* res)
{

    return 0;
}

int GPGPU_API gpgpu_noop(float* a1, float* res)
{
    int ret = 0;
    if (g_helper.state != READY)
        ERR("Call gpgpu_init() first!");

    unsigned char* buffer = malloc(4 * g_helper.width * g_helper.height);
    GLuint texId0, texId1;
    gpgpu_make_texture(a1, g_helper.width, g_helper.height, &texId0);

#if DEBUG
    printf("RAW contents before addition: \n");
    for (int i = 0; i < 4 * g_helper.width * g_helper.height; ++i)
    {
        printf("%d ", *((unsigned char*)a1 + i));
        if ((i + 1)  % (4 * g_helper.width) == 0)
            printf("\n");
    }
    printf("\n");
#endif

    gpgpu_build_program(REGULAR, NOOP);

    GLuint geometry;
    glGenBuffers(1, &geometry);
    glBindBuffer(GL_ARRAY_BUFFER, geometry);
    glBufferData(GL_ARRAY_BUFFER, 20*sizeof(float), gpgpu_geometry, GL_STATIC_DRAW);

    gpgpu_add_attribute("position", 3, 20, 0);
    gpgpu_add_attribute("texCoord", 2, 20, 3);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texId0);
    gpgpu_add_uniform("texture0", 0, "uniform1i");

    glActiveTexture(GL_TEXTURE0);

    if (gpgpu_report_glError(glGetError()) != 0)
        ERR("Could not prepare textures");

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glReadPixels(0, 0, g_helper.width, g_helper.height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

#if DEBUG
    printf("RAW contents after addition: \n");
    for (int i = 0; i < 4 * g_helper.width * g_helper.height; ++i)
    {
        printf("%d ", buffer[i]);
        if ((i + 1)  % (4 * g_helper.width) == 0)
            printf("\n");
    }
    printf("\n");
#endif

    // copy the bytes as floats
    for (int i = 0; i < 4 * g_helper.width * g_helper.height; i += 4)
    {
        res[i / 4] = *((float*)buffer + i / 4);
    }

bail:
    if (buffer)
        free(buffer);
    return ret;
}

int GPGPU_API gpgpu_arrayAddition_fixed16(uint16_t* a1, uint16_t* a2, uint16_t* res, uint8_t fractional_bits)
{
    int ret = 0;
    if (g_helper.state != READY)
        ERR("Call gpgpu_init() first!");

    int fraction_divider = 1 << fractional_bits;
    unsigned char* buffer = malloc(4 * g_helper.width * g_helper.height);
    GLuint texId0, texId1;
    gpgpu_make_texture(a1, g_helper.width, g_helper.height, &texId0);
    gpgpu_make_texture(a2, g_helper.width, g_helper.height, &texId1);

#if DEBUG
    printf("RAW contents before addition: \n");
    for (int i = 0; i < 4 * g_helper.width * g_helper.height; ++i)
    {
        printf("%d ", *((unsigned char*)a1 + i));
        if ((i + 1)  % (4 * g_helper.width) == 0)
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

    glReadPixels(0, 0, g_helper.width, g_helper.height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    // convert from unsigned bytes back to the original format

#if DEBUG
    printf("RAW contents after addition: \n");
    for (int i = 0; i < 4 * g_helper.width * g_helper.height; ++i)
    {
        printf("%d ", buffer[i]);
        if ((i + 1)  % (4 * g_helper.width) == 0)
            printf("\n");
    }
    printf("\n");
#endif

    // copy the bytes as floats
    for (int i = 0; i < 4 * g_helper.width * g_helper.height; i += 2)
    {
        res[i / 2] = *((uint16_t*)buffer + i / 2);
    }

bail:
    if (buffer)
        free(buffer);
    return ret;
}

///////////////////////////////
////* CHAIN API FUNCTIONS *////
///////////////////////////////

int GPGPU_API gpgpu_chain_apply_float(EOperation* operations, UOperationPayloadFloat* payload, int len, float* a1, float* res)
{
    int ret = 0;
    if (g_helper.state != READY)
        ERR("Call gpgpu_init() first!");

    gpgpu_make_texture(a1, g_helper.width, g_helper.height, &g_chainHelper.in_texId0);
    // generate a double-buffer texture, which will hold the contents of FB for swapping around
    // texId0 is the regular FBO, texId1 contains the copy of last FBO
    gpgpu_make_texture(NULL, g_helper.width, g_helper.height, &g_chainHelper.output_texId1);

#if DEBUG
    printf("RAW contents before addition: \n");
    for (int i = 0; i < 4 * g_helper.width * g_helper.height; ++i)
    {
        printf("%d ", *((unsigned char*)a1 + i));
        if ((i + 1)  % (4 * g_helper.width) == 0)
            printf("\n");
    }
    printf("\n");
#endif

    g_helper.state = COMPUTING;

    for (int i = 0; i < len; ++i)
    {
        switch(operations[i])
        {
            case ADD_SCALAR_FLOAT:
                if (gpgpu_chain_op_scalar_float(payload[i].s, ADD) != 0)
                    ERR("Error calling chain add!");
                break;
            case SUBTRACT_SCALAR_FLOAT:
                if (gpgpu_chain_op_scalar_float(payload[i].s, SUBTRACT) != 0)
                    ERR("Error calling chain subtract!");
                break;
            case MULTIPLY_SCALAR_FLOAT:
                if (gpgpu_chain_op_scalar_float(payload[i].s, MULTIPLY) != 0)
                    ERR("Error calling chain multiply!");
                break;
            case DIVIDE_SCALAR_FLOAT:
                if (gpgpu_chain_op_scalar_float(payload[i].s, DIVIDE) != 0)
                    ERR("Error calling chain divide!");
                break;
            case FIR_CONV2D_FLOAT:
                // consume two operation slots
                if (gpgpu_chain_conv2d_float(payload[i].arr, payload[i + 1].n) != 0)
                    ERR("Error calling gpgpu_chain_add_scalar_float!");
                ++i;
                break;
        }
        // switch the textures to make previous output next input and so on
        if (i != len -1) // skip for last operation (TODO: will it work with double width operations? probably no!)
        {
            //printf("swapped\n"); // TODO: debug message
            gpgpu_copy_FBO_output();
        }
    }

    if (gpgpu_chain_finish_float(res) != 0)
        ERR("Could not return the computed data!");
bail:
    g_helper.state = READY;
    return ret;
}

int GPGPU_API gpgpu_chain_finish_float(float* res)
{
    int ret = 0;
    unsigned char* buffer = malloc(4 * g_helper.width * g_helper.height);
    if (g_helper.state != COMPUTING)
        ERR("This can only be called via the chain_apply functions!");

    // it will read from the currently bound output texture
    glReadPixels(0, 0, g_helper.width, g_helper.height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

#if DEBUG
    printf("RAW contents after addition: \n");
    for (int i = 0; i < 4 * g_helper.width * g_helper.height; ++i)
    {
        printf("%d ", buffer[i]);
        if ((i + 1)  % (4 * g_helper.width) == 0)
            printf("\n");
    }
    printf("\n");
#endif

    // copy the bytes as floats
    for (int i = 0; i < 4 * g_helper.width * g_helper.height; i += 4)
    {
        res[i / 4] = *((float*)buffer + i / 4);
    }
bail:
    free(buffer);
    return ret;
}

int GPGPU_API gpgpu_chain_op_scalar_float(float s, EArithmeticOperator op)
{
    int ret = 0;
    if (g_helper.state != COMPUTING)
        ERR("This can only be called via the chain_apply functions!");

    switch (op) {
        case ADD:
            gpgpu_build_program(REGULAR, CHAIN_ADD_SCALAR_FLOAT);
            break;
        case SUBTRACT:
            gpgpu_build_program(REGULAR, CHAIN_SUBTRACT_SCALAR_FLOAT);
            break;
        case MULTIPLY:
            gpgpu_build_program(REGULAR, CHAIN_MULTIPLY_SCALAR_FLOAT);
            break;
        case DIVIDE:
            if (s == 0.0)
                ERR("Cannot divide by 0!");
            gpgpu_build_program(REGULAR, CHAIN_DIVIDE_SCALAR_FLOAT);
            break;
    }

    GLuint geometry;
    glGenBuffers(1, &geometry);
    glBindBuffer(GL_ARRAY_BUFFER, geometry);
    glBufferData(GL_ARRAY_BUFFER, 20*sizeof(float), gpgpu_geometry, GL_STATIC_DRAW);

    gpgpu_add_attribute("position", 3, 20, 0);
    gpgpu_add_attribute("texCoord", 2, 20, 3);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_chainHelper.in_texId0);
    gpgpu_add_uniform("texture0", 0, "uniform1i");

    gpgpu_add_uniform("scalar", s, "uniform1f");

    glActiveTexture(GL_TEXTURE0);

    if (gpgpu_report_glError(glGetError()) != 0)
        ERR("Could not prepare textures");

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // now the currently bound out_texture contains the result
bail:
    return ret;
}

/* ADD OPS and maybe move to a different source file */

int GPGPU_API gpgpu_chain_conv2d_float(float* kernel, int size)
{
    int ret = 0;
    if (g_helper.state != COMPUTING)
        ERR("This can only be called via the chain_apply functions!");

    gpgpu_make_texture(kernel, size, size, &g_chainHelper.in_texId1);
#ifndef BEAGLE
        switch (size)
        {
            case 3:
                gpgpu_build_program(REGULAR, FIR_CONV2D_FLOAT_3);
                break;
            case 5:
                if (g_helper.width < 5 || g_helper.height < 5)
                    ERR("WIDTH && HEIGHT must be greater than 5");
                gpgpu_build_program(REGULAR, FIR_CONV2D_FLOAT_5);
                break;
            default:
                ERR("Supported kernel size: 3, 5");
        }
#else
        switch (size)
        {
            case 3:
                gpgpu_build_program(REGULAR, FIR_CONV2D_FLOAT_BBB_3);
                break;
            case 5:
                if (g_helper.width < 5 || g_helper.height < 5)
                    ERR("WIDTH && HEIGHT must be greater than 5");
                gpgpu_build_program(REGULAR, FIR_CONV2D_FLOAT_BBB_5);
                break;
            default:
                ERR("Supported kernel size: 3, 5");
        }
#endif

    GLuint geometry;
    glGenBuffers(1, &geometry);
    glBindBuffer(GL_ARRAY_BUFFER, geometry);
    glBufferData(GL_ARRAY_BUFFER, 20*sizeof(float), gpgpu_geometry, GL_STATIC_DRAW);

    gpgpu_add_attribute("position", 3, 20, 0);
    gpgpu_add_attribute("texCoord", 2, 20, 3);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_chainHelper.in_texId0);
    gpgpu_add_uniform("texture0", 0, "uniform1i");

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, g_chainHelper.in_texId1);
    gpgpu_add_uniform("texture1", 1, "uniform1i");

    // add the texture step =  1 / width
    gpgpu_add_uniform("w", 1.0 / g_helper.width, "uniform1f");

    glActiveTexture(GL_TEXTURE0);

    if (gpgpu_report_glError(glGetError()) != 0)
        ERR("Could not prepare textures");

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // now the currently bound out_texture contains the result
bail:
    return ret;
}
