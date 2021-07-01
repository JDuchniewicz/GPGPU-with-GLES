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

    g_helper.context = eglCreateContext(g_helper.display, g_helper.config, EGL_NO_CONTEXT, NULL);
    if (g_helper.context == EGL_NO_CONTEXT)
        ERR("Could not create EGL context");

    if (eglMakeCurrent(g_helper.display, g_helper.surface, g_helper.surface, g_helper.context) != EGL_TRUE) // EGL_NO_SURFACE??
        ERR("Could not bind the surface to context");

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

int GPGPU_API gpgpu_arrayAddition(int* a1, int* a2, int len, int* res)
{
    int ret = 0;
    GLuint texId0, texId1;
    gpgpu_make_texture(a1, WIDTH, HEIGHT, &texId0);
    gpgpu_make_texture(a2, WIDTH, HEIGHT, &texId1);

    const GLchar* fragmentSource = "precision mediump float;\n"
                                   "uniform mediump sampler2D texture0;\n"
                                   "uniform mediump sampler2D texture1;\n"
                                   "varying vec2 vTexCoord;\n"
                                   "void main(void) {\n"
                                   "vec4 pixel1 = texture2D(texture0, vTexCoord);\n"
                                   "vec4 pixel2 = texture2D(texture1, vTexCoord);\n"
                                   "gl_FragColor.r = pixel1.r + pixel2.r;\n"
                                   "gl_FragColor.g = pixel1.g + pixel2.g;\n"
                                   "gl_FragColor.b = pixel1.b + pixel2.b;\n"
                                   "gl_FragColor.a = pixel1.a + pixel2.a;\n"
                                   "}\n";

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

    // magic happens and the data is now ready
    // poof!
    //////////

    glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, res);

    return ret;
bail:
    // TODO: what should be released upon failure?
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

    ret = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (ret != GL_FRAMEBUFFER_COMPLETE)
        gpgpu_report_framebuffer_status(ret);
    else
        ret = 0;

    return ret;
}

static void gpgpu_make_texture(int* buffer, int w, int h, GLuint* texId) //TODO: int to float casting?
{
    glGenTextures(1, texId);
    glBindTexture(GL_TEXTURE_2D, *texId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
}

static void gpgpu_build_program(const GLchar* vertexSource, const GLchar* fragmentSource)
{
    int infoLen = 0;
    // TODO: shader management code (storing them in files etc)
    // compile shaders
    // vertex
    printf("BIP\n");
    gpgpu_report_glError(glGetError());
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    printf("BOP\n");
    gpgpu_report_glError(glGetError());
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
    gpgpu_report_glError(glGetError());
    g_helper.ESShaderProgram = glCreateProgram();
    printf("%d\n", g_helper.ESShaderProgram);
    gpgpu_report_glError(glGetError());

    // attach them
    glAttachShader(g_helper.ESShaderProgram, vertex);
    glAttachShader(g_helper.ESShaderProgram, fragment);

    glLinkProgram(g_helper.ESShaderProgram);
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
    if (strcmp(type, "uniform1f"))
        glUniform1f(loc, value);
    else if (strcmp(type, "uniform1i"))
        glUniform1i(loc, value);
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
