#pragma once

#include "defines.h"

// the geometry is scaled to a square 2x2 from (-1,-1) to (1,1)
static const float gpgpu_geometry[20] = {
    -1.0,  1.0, 0.0, 0.0, 1.0, // top left
    -1.0, -1.0, 0.0, 0.0, 0.0, // bottom left
     1.0,  1.0, 0.0, 1.0, 1.0, // top right
     1.0, -1.0, 0.0, 1.0, 0.0  // bottom right
};

int GPGPU_API gpgpu_init(int width, int height);
int GPGPU_API gpgpu_deinit();

// FLOAT FUNCTIONS
int GPGPU_API gpgpu_arrayAddition(float* a1, float* a2, float* res);
int GPGPU_API gpgpu_firConvolution2D(float* data, float* kernel, int size, float* res);
int GPGPU_API gpgpu_matrixMultiplication(int* a, int* b, int size, int* res);
// noop
int GPGPU_API gpgpu_noop(float* a1, float* res);

// 16-bit functions
// 16-bit fixed-point functions
int GPGPU_API gpgpu_arrayAddition_fixed16(uint16_t* a1, uint16_t* a2, uint16_t* res, uint8_t fractional_bits);

// chaining API
// needs chaining via an output texture instead of a framebuffer
int GPGPU_API gpgpu_chain_apply_float(EOperation* operations, UOperationPayloadFloat* payload, int len, float* a1, float* res);
int GPGPU_API gpgpu_chain_finish_float(float* res);

// broadcast operations
int GPGPU_API gpgpu_chain_op_scalar_float(float s, EArithmeticOperator op);

int GPGPU_API gpgpu_chain_op_array_float(float* arr, EArithmeticOperator op);

int GPGPU_API gpgpu_chain_conv2d_float(float* kernel, int size);
