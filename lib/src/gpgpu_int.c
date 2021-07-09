#include "gpgpu_int.h"
#include "debug.h"

extern GLHelper g_helper;
/////////////////////////////
////* PRIVATE FUNCTIONS *////
/////////////////////////////

int gpgpu_check_egl_extensions()
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

int gpgpu_find_matching_config(EGLConfig* config, uint32_t gbm_format)
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

        if (!eglGetConfigAttrib(g_helper.display, configs[i], EGL_CONFIG_ID, &format)) // TODO: should be matched in a more robust way
            ERR("Could not iterate through configs");

	printf("EGL format: %d Seeked: %d\n", format, gbm_format);
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

int gpgpu_make_FBO(int w, int h)
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

void gpgpu_make_texture(float* buffer, int w, int h, GLuint* texId) //TODO: int to float casting?
{
    glGenTextures(1, texId);
    glBindTexture(GL_TEXTURE_2D, *texId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // openGL loads the texture from bottom to top, left to right so the data is in wrong order
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer); // read floats, treat them as unsigned bytes
}

int gpgpu_build_program(EVertexShader vertType, EFragmentShader fragType)
{
    int ret = 0, infoLen = 0;
    GLchar* vShader = NULL;
    GLchar* fShader = NULL;
    if (gpgpu_load_shaders(vertType, fragType, &vShader, &fShader) != 0)
        ERR("Could not load shader code");

    g_helper.ESShaderProgram = glCreateProgram();
    // compile shaders
    // vertex
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShader, NULL);
    glCompileShader(vertex);

    glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen > 1) // TODO: optimize it
    {
        // allocate on stack for now
        char outLog[infoLen];
        glGetShaderInfoLog(vertex, infoLen, NULL, outLog);
        printf("VERTEX:\n %s\n", outLog);
        ERR("Could not create Vertex shader");
    }

    // fragment
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShader, NULL);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen > 1) // TODO: optimize it
    {
        // allocate on stack for now
        char outLog[infoLen];
        glGetShaderInfoLog(fragment, infoLen, NULL, outLog);
        printf("FRAGMENT:\n %s\n", outLog);
        ERR("Could not create Fragment shader");
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
            ERR("Could not link shader");
        }
    }
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    glUseProgram(g_helper.ESShaderProgram);

bail:
    if (vShader)
        free(vShader);
    if (fShader)
        free(fShader);
    return ret;
}

void gpgpu_add_attribute(const char* name, int size, int stride, int offset)
{
    // setup the vertex position as the attribute of vertex shader
    glUseProgram(g_helper.ESShaderProgram);
    int loc = glGetAttribLocation(g_helper.ESShaderProgram, name);
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, size, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(offset * sizeof(float)));
}

void gpgpu_add_uniform(const char* name, float value, const char* type)
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
