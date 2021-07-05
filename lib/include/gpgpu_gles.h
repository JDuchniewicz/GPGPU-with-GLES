#pragma once

#include "defines.h"

// the geometry is scaled to a square 2x2 from (-1,-1) to (1,1)
static const float gpgpu_geometry[20] = {
    -1.0,  1.0, 0.0, 0.0, 1.0, // top left
    -1.0, -1.0, 0.0, 0.0, 0.0, // bottom left
     1.0,  1.0, 0.0, 1.0, 1.0, // top right
     1.0, -1.0, 0.0, 1.0, 0.0  // bottom right
};

static const GLchar* RegularVShader = "attribute vec3 position;\n"
                                      "attribute vec2 texCoord;\n"
                                      "varying highp vec2 vTexCoord;\n"
                                      "void main(void) {\n"
                                      "gl_Position = vec4(position, 1.0);\n"
                                      "vTexCoord = texCoord;\n"
                                      "}\n";

int GPGPU_API gpgpu_init();
int GPGPU_API gpgpu_deinit();
int GPGPU_API gpgpu_arrayAddition(float* a1, float* a2, float* res);
int GPGPU_API gpgpu_firConvolution(int* data, int len, int* kernel, int size, int* res);
int GPGPU_API gpgpu_matrixMultiplication(int* a, int* b, int size, int* res);

