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

#define DEBUG 0 // set it via debug build flag

#define ERR(m) { \
    ret = -1; \
    fprintf(stderr, "ERROR in file %s line %d: %s\n", __FILE__ ,__LINE__, m); \
    goto bail; \
}

typedef enum
{
    // float
    // scalar
    ADD_SCALAR_FLOAT,
    // a x a
    //
    FIR_CONV2D_FLOAT,
} EOperation;

typedef union
{
    float s;
    float* arr;
    int n; // kernel size
} UOperationPayloadFloat;

typedef enum
{
    INIT = 0,
    READY,
    COMPUTING,
} EProgramState;

typedef struct
{
    EProgramState state;
	GLint ESShaderProgram;
	EGLDisplay display;
	EGLConfig config;
	EGLContext context;
    EGLSurface surface;
#ifndef BEAGLE
    struct gbm_device* gbm;
    struct gbm_surface* gbm_surface;
    int32_t gbd_fd;
#endif
    int height;
    int width;
} GLHelper;

typedef struct
{
    GLuint in_texId0, in_texId1;
    GLuint output_texId0, output_texId1;
    GLuint fbId;
} GChainHelper;
