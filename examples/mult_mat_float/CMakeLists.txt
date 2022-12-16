cmake_minimum_required(VERSION 3.7)
project(GPGPUGLESMatrixMultiplyFloat VERSION 1.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(GPGPU_GLES REQUIRED)

add_executable(GPGPUGLESMatrixMultiplyFloat
        ${CMAKE_CURRENT_LIST_DIR}/mult_mat_float.c)

target_link_libraries(GPGPUGLESMatrixMultiplyFloat PRIVATE GPGPU_GLES::GPGPU_GLES)
