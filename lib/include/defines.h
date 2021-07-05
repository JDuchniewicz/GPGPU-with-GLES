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

#define DEBUG 0

// TODO: this needs tweaking
#define WIDTH 4
#define HEIGHT 4

#define ERR(m) { \
    ret = -1; \
    fprintf(stderr, "ERROR in line %d: %s\n", __LINE__, m); \
    goto bail; \
}

typedef struct
{
	GLint ESShaderProgram;
	GLbyte* FShaderSource; //the shader programs are specified as strings and are loaded depending on the program (maybe should be binary blobs in the final impl)
	GLbyte* VShaderSource; //ditto
	EGLDisplay display;
	EGLConfig config;
	EGLContext context;
    EGLSurface surface;
    struct gbm_device* gbm;
    struct gbm_surface* gbm_surface;
    int32_t gbd_fd;
} GLHelper;
