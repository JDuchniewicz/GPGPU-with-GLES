#pragma once
#include <stdint.h>

void array_add_float();
void array_add_fixed16();

void cpu_compute_array_add_float(float* a1, float* a2, float* res);
void generate_data_f(float* in1, float* in2);
void print_data_f(float* a);

void cpu_compute_array_add_fixed16(uint16_t* a1, uint16_t* a2, uint16_t* res);
void generate_data_f16(uint16_t* in1, uint16_t* in2);
void print_data_f16(uint16_t* a);
