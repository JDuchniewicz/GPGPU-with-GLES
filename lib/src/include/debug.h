#pragma once

#include "defines.h"

// private logging functions
void gpgpu_report_framebuffer_status(int ret);
void dumpEGLconfig(EGLConfig *EGLConfig, EGLDisplay display);
int gpgpu_report_glError(GLenum error);
