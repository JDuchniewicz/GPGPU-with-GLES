#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>

#include <EGL/egl.h>
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>
#include <GLES2/gl2.h> // what about GLES3?

#define GPGPU_API // just a marker

typedef struct
{
	GLint ESShaderProgram;
	GLbyte* FShaderSource; //the shader programs are specified as strings and are loaded depending on the program (maybe should be binary blobs in the final impl)
	GLbyte* VShaderSource; //ditto
	EGLDisplay display;
	EGLSurface surface;
	EGLint config;
	EGLContext context;
} GLHelper;

int GPGPU_API gpgpu_init();
int GPGPU_API gpgpu_deinit();
int GPGPU_API gpgpu_arrayAddition(int* a1, int* a2, int len, int* res);
int GPGPU_API gpgpu_firConvolution(int* data, int len, int* kernel, int size, int* res);
int GPGPU_API gpgpu_matrixMultiplication(int* a, int* b, int size, int* res);
