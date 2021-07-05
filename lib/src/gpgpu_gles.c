#include "gpgpu_gles.h"

GLHelper g_helper;

#define ERR(m) { \
    ret = -1; \
    fprintf(stderr, "ERROR in line %d: %s\n", __LINE__, m); \
    goto bail; \
}

// TODO: map GL errors?

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

    printf("RAW contents before addition: \n");
    for (int i = 0; i < 4 * WIDTH * HEIGHT; ++i)
    {
        printf("%d ", *((unsigned char*)a1 + i));
        if ((i + 1)  % (4 * WIDTH) == 0)
            printf("\n");
    }
    printf("\n");

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
                                   " value = sgn * exp2(exponent) * (1.0 + mantissa * exp2(-23.0));"
                                   ""
                                   " return value;"
                                   "}"
                                   ""
                                   "void main(void)"
                                   "{"
                                   " vec4 texel1 = texture2D(texture0, vTexCoord);"
                                   " vec4 texel2 = texture2D(texture1, vTexCoord);"
                                   //" float a1 = unpack(texel1);"
                                   //" float a2 = unpack(texel2);"
                                   //" gl_FragColor = pack(a1 + a2);"
                                   " gl_FragColor = texel1;"
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
    printf("RAW contents after addition: \n");
    for (int i = 0; i < 4 * WIDTH * HEIGHT; ++i)
    {
        printf("%d ", buffer[i]);
        if ((i + 1)  % (4 * WIDTH) == 0)
            printf("\n");
    }
    printf("\n");

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

/////////////////////////////
////* PRIVATE FUNCTIONS *////
/////////////////////////////

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
        ERR("Could not get number of configs");

    EGLConfig* configs = malloc(count * sizeof(EGLConfig));
    if (!eglChooseConfig(g_helper.display, config_attrs, configs, count, &count) || count < 1)
        ERR("Could not choose configs or config size < 1");

    printf("Seeked ID %d\n", gbm_format);
    for (int i = 0; i < count; ++i)
    {
        EGLint format;

        if (!eglGetConfigAttrib(g_helper.display, configs[i], EGL_NATIVE_VISUAL_ID, &format))
            ERR("Could not iterate through configs");

        dumpEGLconfig(configs[i], g_helper.display);
        if (gbm_format == format)
        {
            *config = configs[i];
            free(configs);
            return ret;
        }
    }

    ERR("Failed to find a matching config");
bail:
    if (configs)
        free(configs);
    return ret;
}

void dumpEGLconfig(EGLConfig *EGLConfig, EGLDisplay display)
{
	EGLint value;

 	eglGetConfigAttrib(display,EGLConfig,EGL_BUFFER_SIZE,&value);
	printf("Buffer Size %i\n", value);
	eglGetConfigAttrib(display,EGLConfig,EGL_RED_SIZE,&value);
	printf("Red Size %i\n", value);
	eglGetConfigAttrib(display,EGLConfig,EGL_GREEN_SIZE,&value);
	printf("Green Size %i\n", value);
	eglGetConfigAttrib(display,EGLConfig,EGL_BLUE_SIZE,&value);
	printf("Blue Size %i\n", value);
	eglGetConfigAttrib(display,EGLConfig,EGL_ALPHA_SIZE,&value);
	printf("Alpha Size %i\n", value);
	eglGetConfigAttrib(display,EGLConfig,EGL_CONFIG_CAVEAT,&value);
	switch(value) {
	case  EGL_NONE:
		printf("EGL_CONFIG_CAVEAT EGL_NONE\n");
		break;
	case  EGL_SLOW_CONFIG:
		printf("EGL_CONFIG_CAVEAT EGL_SLOW_CONFIG\n");
		break;
	}
	eglGetConfigAttrib(display,EGLConfig,EGL_CONFIG_ID,&value);
	printf("Config ID %i\n", value);

	eglGetConfigAttrib(display,EGLConfig,EGL_DEPTH_SIZE,&value);
	printf("Depth size %i\n", value);

	eglGetConfigAttrib(display,EGLConfig,EGL_MAX_PBUFFER_WIDTH,&value);
	printf("Max pbuffer width %i\n", value);
	eglGetConfigAttrib(display,EGLConfig,EGL_MAX_PBUFFER_HEIGHT,&value);
	printf("Max pbuffer height %i\n", value);
	eglGetConfigAttrib(display,EGLConfig,EGL_MAX_PBUFFER_PIXELS,&value);
	printf("Max pbuffer pixels %i\n", value);
	eglGetConfigAttrib(display,EGLConfig,EGL_NATIVE_RENDERABLE,&value);
	printf("Native renderable %s\n", (value ? "true" : "false"));
	eglGetConfigAttrib(display,EGLConfig,EGL_NATIVE_VISUAL_ID,&value);
	printf("Native visual ID %i\n", value);
	eglGetConfigAttrib(display,EGLConfig,EGL_NATIVE_VISUAL_TYPE,&value);
	printf("Native visual type %i\n", value);
	eglGetConfigAttrib(display,EGLConfig,EGL_SAMPLE_BUFFERS,&value);
	printf("Sample Buffers %i\n", value);
	eglGetConfigAttrib(display,EGLConfig,EGL_SAMPLES,&value);
	printf("Samples %i\n", value);
	eglGetConfigAttrib(display,EGLConfig,EGL_SURFACE_TYPE,&value);
	printf("Surface type %i\n", value);
	eglGetConfigAttrib(display,EGLConfig,EGL_TRANSPARENT_TYPE,&value);
	printf("Transparent type %i\n", value);
	eglGetConfigAttrib(display,EGLConfig,EGL_RENDERABLE_TYPE,&value);
	printf("Renderable type %i\n", value);
	printf("-------------------------------------\n");
}

static int gpgpu_make_FBO(int w, int h)
{
    int ret = 0;
    GLuint texId, fbId;

    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0); // allows for floating-point buffer in ES2.0 (format should be RGBA32F)
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &fbId);
    glBindFramebuffer(GL_FRAMEBUFFER, fbId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
    // magic trick!
    glViewport(0, 0, WIDTH, HEIGHT);

    ret = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (ret != GL_FRAMEBUFFER_COMPLETE)
        gpgpu_report_framebuffer_status(ret);
    else
        ret = 0;

    return ret;
}

static void gpgpu_make_texture(float* buffer, int w, int h, GLuint* texId) //TODO: int to float casting?
{
    //unsigned char fakea1[4] = {0, 1, 2, 3};
    //unsigned char fakea1[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    //unsigned char fakea1[64] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,\
                                16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,\
                                32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,\
                                48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63};
    glGenTextures(1, texId);
    glBindTexture(GL_TEXTURE_2D, *texId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // openGL loads the texture from bottom to top, left to right so the data is in wrong order
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer); // read floats, treat them as unsigned bytes
}

static void gpgpu_build_program(const GLchar* vertexSource, const GLchar* fragmentSource)
{
    int infoLen = 0;
    g_helper.ESShaderProgram = glCreateProgram();
    // TODO: shader management code (storing them in files etc)
    // compile shaders
    // vertex
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSource, NULL);
    glCompileShader(vertex);

    glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen > 1) // TODO: optimize it
    {
        // allocate on stack for now
        char outLog[infoLen];
        glGetShaderInfoLog(vertex, infoLen, NULL, outLog);
        printf("VERTEX:\n %s\n", outLog);
    }

    // fragment
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSource, NULL);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen > 1) // TODO: optimize it
    {
        // allocate on stack for now
        char outLog[infoLen];
        glGetShaderInfoLog(fragment, infoLen, NULL, outLog);
        printf("FRAGMENT:\n %s\n", outLog);
    }

    // attach them
    glAttachShader(g_helper.ESShaderProgram, vertex);
    glAttachShader(g_helper.ESShaderProgram, fragment);

    glLinkProgram(g_helper.ESShaderProgram);

    GLint linked;
    glGetProgramiv(g_helper.ESShaderProgram, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        glGetProgramiv(g_helper.ESShaderProgram, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1)
        {
            // allocate on stack for now
            char outLog[infoLen];
            glGetProgramInfoLog(g_helper.ESShaderProgram, infoLen, NULL, outLog);
            printf("PROGRAM:\n %s\n", outLog);
        }
    }
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    glUseProgram(g_helper.ESShaderProgram);
}

static void gpgpu_add_attribute(const char* name, int size, int stride, int offset)
{
    // setup the vertex position as the attribute of vertex shader
    glUseProgram(g_helper.ESShaderProgram);
    int loc = glGetAttribLocation(g_helper.ESShaderProgram, name);
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, size, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(offset * sizeof(float)));
}

static void gpgpu_add_uniform(const char* name, int value, const char* type)
{
    int loc = glGetUniformLocation(g_helper.ESShaderProgram, name);
    if (strcmp(type, "uniform1f") == 0)
        glUniform1f(loc, value);
    else if (strcmp(type, "uniform1i") == 0)
    {
        glUniform1i(loc, value);
    }
    else
        printf("UNKNOWN UNIFORM\n");
}

static void gpgpu_report_framebuffer_status(int ret)
{
    switch(ret)
    {
        case GL_FRAMEBUFFER_UNSUPPORTED:
            printf("FRAMEBUFFER_UNSUPPORTED\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            printf("FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
            printf("FRAMEBUFFER_INCOMPLETE_DIMENSIONS\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            printf("FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n");
            break;
        default:
            printf("Framebuffer creation error %d\n", ret);
            break;
    }
}

static int gpgpu_report_glError(GLenum error)
{
    int ret = 0;
    while (error != GL_NO_ERROR)
    {
        ret = -1;
        switch (error) {
            case GL_INVALID_ENUM:
                printf("INVALID_ENUM\n");
                break;
            case GL_INVALID_VALUE:
                printf("INVALID_VALUE\n");
                break;
            case GL_INVALID_OPERATION:
                printf("INVALID_OPERATION\n");
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                printf("INVALID_FRAMEBUFFER_OPERATION\n");
                break;
            case GL_OUT_OF_MEMORY:
                printf("OUT_OF_MEMORY\n");
                break;
            default:
                printf("GL UNKOWN ERROR!\n");
                break;
        }
        error = glGetError();
    }
    return ret;
}
