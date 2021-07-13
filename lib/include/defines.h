#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <fcntl.h>
#include <gbm/gbm.h>

#include <EGL/egl.h>
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>
#include <GLES2/gl2.h> // what about GLES3?

#define GPGPU_API // just a marker

#define DEBUG 1 // set it via debug build flag

#define ERR(m) { \
    ret = -1; \
    fprintf(stderr, "ERROR in file %s line %d: %s\n", __FILE__ ,__LINE__, m); \
    goto bail; \
}

typedef struct
{
	GLint ESShaderProgram;
	EGLDisplay display;
	EGLConfig config;
	EGLContext context;
    EGLSurface surface;
    struct gbm_device* gbm;
    struct gbm_surface* gbm_surface;
    int32_t gbd_fd;
    int height;
    int width;
} GLHelper;
