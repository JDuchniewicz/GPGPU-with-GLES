#pragma once
#include <stdint.h>

void array_add_float();
void array_add_fixed16();
void conv2d_float(int size);

void chain_add_float(int rep);
void chain_conv2d_float(int rep, int size);

void cpu_compute_array_add_float(float* a1, float* a2, float* res);
void generate_data_f(float* in1, float* in2);
void print_data_f(float* a);

void cpu_compute_array_add_fixed16(uint16_t* a1, uint16_t* a2, uint16_t* res);
void generate_data_f16(uint16_t* in1, uint16_t* in2);
void print_data_f16(uint16_t* a);

void cpu_compute_conv2d_float(float* a1, float* kernel, int size, float* res);
void generate_data_conv2d_f(float* in1);

void cpu_compute_chain_add_float(int rep, float scalar, float* a1, float* res);
void cpu_compute_chain_conv2d_float(int rep, float* a1, float* kernel, int size, float* res);

void noop();

