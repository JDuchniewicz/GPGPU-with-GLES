#include "debug.h"

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

void gpgpu_report_framebuffer_status(int ret)
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

int gpgpu_report_glError(GLenum error)
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
