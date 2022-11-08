#pragma once

#include "defines.h"

typedef enum
{
    ARRAY_ADD_FLOAT = 0,
    ARRAY_ADD_FIXED16,
    FIR_CONV2D_FLOAT_3,
    FIR_CONV2D_FLOAT_5,
    FIR_CONV2D_FLOAT_BBB_3, // special unrolled version for SGX
    FIR_CONV2D_FLOAT_BBB_5,
    MULT_MAT_INT,
    // chain operations
    CHAIN_ADD_SCALAR_FLOAT,
    CHAIN_SUBTRACT_SCALAR_FLOAT,
    CHAIN_MULTIPLY_SCALAR_FLOAT,
    CHAIN_DIVIDE_SCALAR_FLOAT,
    CHAIN_ADD_ARRAY_FLOAT,
    CHAIN_SUBTRACT_ARRAY_FLOAT,
    CHAIN_MULTIPLY_ARRAY_FLOAT,
    CHAIN_DIVIDE_ARRAY_FLOAT,
    NOOP,
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
    { .type = REGULAR,                      .filename = "../shaders/regular.vs" }, //TODO: fix paths
};

static const struct FFileName fFileNames[] = {
    { .type = ARRAY_ADD_FLOAT,                .filename = "../shaders/array_add_float.fs" },
    { .type = ARRAY_ADD_FIXED16,              .filename = "../shaders/array_add_fixed16.fs" },
    { .type = FIR_CONV2D_FLOAT_3,             .filename = "../shaders/fir_conv2d_float_3.fs" },
    { .type = FIR_CONV2D_FLOAT_5,             .filename = "../shaders/fir_conv2d_float_5.fs" },
    { .type = FIR_CONV2D_FLOAT_BBB_3,         .filename = "../shaders/fir_conv2d_float_bbb_3.fs" },
    { .type = FIR_CONV2D_FLOAT_BBB_5,         .filename = "../shaders/fir_conv2d_float_bbb_5.fs" },
    { .type = MULT_MAT_INT,                   .filename = "../shaders/mult_mat_int.fs" },
    // chain operations
    { .type = CHAIN_ADD_SCALAR_FLOAT,         .filename = "../shaders/chain_add_scalar_float.fs" },
    { .type = CHAIN_SUBTRACT_SCALAR_FLOAT,    .filename = "../shaders/chain_subtract_scalar_float.fs" },
    { .type = CHAIN_MULTIPLY_SCALAR_FLOAT,    .filename = "../shaders/chain_multiply_scalar_float.fs" },
    { .type = CHAIN_DIVIDE_SCALAR_FLOAT,      .filename = "../shaders/chain_divide_scalar_float.fs" },
    { .type = CHAIN_ADD_ARRAY_FLOAT,          .filename = "../shaders/chain_add_array_float.fs" },
    { .type = CHAIN_SUBTRACT_ARRAY_FLOAT,     .filename = "../shaders/chain_subtract_array_float.fs" },
    { .type = CHAIN_MULTIPLY_ARRAY_FLOAT,     .filename = "../shaders/chain_multiply_array_float.fs" },
    { .type = CHAIN_DIVIDE_ARRAY_FLOAT,       .filename = "../shaders/chain_divide_array_float.fs" }, // TODO: question can we unify the regular shaders with the chain ones?
    { .type = NOOP,                           .filename = "../shaders/noop.fs" },
};

int gpgpu_load_shaders(EVertexShader vertType, EFragmentShader fragType, GLchar** vSource, GLchar** fSource);
