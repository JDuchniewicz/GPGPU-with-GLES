#include "gpgpu_gles.h"
#include "gpgpu_int.h"
#include "debug.h"

GLHelper g_helper;

/////////////////////////
////* API FUNCTIONS *////
/////////////////////////

int GPGPU_API gpgpu_init(uint32_t height, uint32_t width)
{
    int ret = 0;
    int major, minor;

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

    if (gpgpu_find_matching_config(&g_helper.config, GBM_FORMAT_ARGB8888) != 0)
        ERR("Could not find matching config");

    if (eglBindAPI(EGL_OPENGL_ES_API) == 0)
        ERR("Could not bind the API");

    g_helper.gbm_surface = gbm_surface_create(g_helper.gbm, width, height, GBM_FORMAT_ARGB8888, GBM_BO_USE_RENDERING);
    if (!g_helper.gbm_surface)
        ERR("Could not create GBM surface");

    g_helper.surface = eglCreateWindowSurface(g_helper.display, g_helper.config, g_helper.gbm_surface, NULL);
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
    gbm_device_destroy(g_helper.gbm);
    close(g_helper.gbd_fd);
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
    const GLchar* fragmentSource = "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
                                   "precision highp float;\n"
                                   "#else\n"
                                   "precision mediump float;\n"
                                   "#endif\n"
                                   ""
                                   "uniform sampler2D texture0;"
                                   "uniform sampler2D texture1;"
                                   ""
                                   "varying vec2 vTexCoord;"
                                   ""
                                   "vec4 pack(float value)"
                                   "{"
                                   " if (value == 0.0) return vec4(0);"
                                   ""
                                   " float exponent;"
                                   " float mantissa;"
                                   " vec4 result;"
                                   " float sgn;"
                                   ""
                                   " sgn = step(0.0, -value);"
                                   " value = abs(value);"
                                   ""
                                   " exponent = floor(log2(value));"
                                   " mantissa = value * pow(2.0, -exponent) - 1.0;"
                                   " exponent = exponent + 127.0;"
                                   " result = vec4(0);"
                                   ""
                                   " result.a = floor(exponent / 2.0);"
                                   " exponent = exponent - result.a * 2.0;"
                                   " result.a = result.a + 128.0 * sgn;"
                                   ""
                                   " result.b = floor(mantissa * 128.0);"
                                   " mantissa = mantissa - result.b / 128.0;"
                                   " result.b = result.b + exponent * 128.0;"
                                   ""
                                   " result.g = floor(mantissa * 32768.0);"
                                   " mantissa = mantissa - result.g / 32768.0;"
                                   ""
                                   " result.r = floor(mantissa * 8388608.0);"
                                   ""
                                   " return result / 255.0;"
                                   "}"
                                   ""
                                   "float unpack(vec4 texel)"
                                   "{"
                                   " float exponent;"
                                   " float mantissa;"
                                   " float value;"
                                   " float sgn;"
                                   ""
                                   " sgn = -step(128.0, texel.a);"
                                   " texel.a += 128.0 * sgn;"
                                   ""
                                   " exponent = step(128.0, texel.b);"
                                   " texel.b -= exponent * 128.0;"
                                   " exponent += 2.0 * texel.a - 127.0;"
                                   ""
                                   " mantissa = texel.b * 65536.0 + texel.g * 256.0 + texel.r;"
                                   " value = pow(-1.0, sgn) * exp2(exponent) * (1.0 + mantissa * exp2(-23.0));"
                                   ""
                                   " return value;"
                                   "}"
                                   ""
                                   "void main(void)"
                                   "{"
                                   " vec4 texel1 = texture2D(texture0, vTexCoord);"
                                   " vec4 texel2 = texture2D(texture1, vTexCoord);"
                                   " float a1 = unpack(texel1 * 255.0);" // need to rescale it before?
                                   " float a2 = unpack(texel2 * 255.0);"
                                   " gl_FragColor = pack(a1 + a2);"
                                   "}";

    gpgpu_build_program(RegularVShader, fragmentSource);

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

    free(buffer);
    return ret;
bail:
    // TODO: what should be released upon failure?
    free(buffer);
    return ret;
}

int GPGPU_API gpgpu_firConvolution(int* data, int len, int* kernel, int size, int* res)
{

    return 0;
}

int GPGPU_API gpgpu_matrixMultiplication(int* a, int* b, int size, int* res)
{

    return 0;
}

