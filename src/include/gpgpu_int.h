#pragma once

#include "defines.h"
#include "shaders.h"

// private functions
int gpgpu_check_egl_extensions();
int gpgpu_find_matching_config(EGLConfig* config, uint32_t gbm_format);
int gpgpu_make_FBO();
void gpgpu_copy_FBO_output();
void gpgpu_make_texture(void* buffer, int w, int h, GLuint* texId);
int gpgpu_build_program(EVertexShader vertType, EFragmentShader fragType);
void gpgpu_add_attribute(const char* name, int size, int stride, int offset);
void gpgpu_add_uniform(const char* name, float value, const char* type); // ints will convert to floats but not vice versa

