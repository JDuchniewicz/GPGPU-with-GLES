cmake_minimum_required(VERSION 3.7)
project(GPGPUGLESMatrixMultiplyInt VERSION 1.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(GPGPU_GLES REQUIRED)

add_executable(GPGPUGLESMatrixMultiplyInt
        ${CMAKE_CURRENT_LIST_DIR}/mult_mat_int.c)

target_link_libraries(GPGPUGLESMatrixMultiplyInt PRIVATE GPGPU_GLES::GPGPU_GLES)