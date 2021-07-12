#pragma once

#include "defines.h"

typedef enum
{
    ARRAY_ADD_FLOAT = 0,
    ARRAY_ADD_FIXED16,
    FIR_CONV_FLOAT,
    MAT_MULT_FLOAT,
    NONE = -1,
} EFragmentShader;

typedef enum
{
    REGULAR = 0,
} EVertexShader;

struct VFileName
{
    EVertexShader type;
    const char* filename;
};

struct FFileName
{
    EFragmentShader type;
    const char* filename;
};

static const struct VFileName vFileNames[] = {
    { .type = REGULAR,          .filename = "../shaders/regular.vs" }, //TODO: fix paths
};

static const struct FFileName fFileNames[] = {
    { .type = ARRAY_ADD_FLOAT,   .filename = "../shaders/array_add_float.fs" },
    { .type = ARRAY_ADD_FIXED16, .filename = "../shaders/array_add_fixed16.fs" },
    { .type = FIR_CONV_FLOAT,    .filename = "../shaders/fir_conv_float.fs" },
    { .type = MAT_MULT_FLOAT,    .filename = "mat_mult_float.fs" },
};

int gpgpu_load_shaders(EVertexShader vertType, EFragmentShader fragType, GLchar** vSource, GLchar** fSource);
