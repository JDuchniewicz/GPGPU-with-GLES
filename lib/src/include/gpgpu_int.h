#pragma once

#include "defines.h"

// private functions
int gpgpu_check_egl_extensions();
int gpgpu_find_matching_config(EGLConfig* config, uint32_t gbm_format);
int gpgpu_make_FBO(int w, int h);
void gpgpu_make_texture(float* buffer, int w, int h, GLuint* texId);
void gpgpu_build_program(const GLchar* vertexSource, const GLchar* fragmentSource);
void gpgpu_add_attribute(const char* name, int size, int stride, int offset);
void gpgpu_add_uniform(const char* name, int value, const char* type);

