cmake_minimum_required(VERSION 3.7)
project(GPGPUGLESChainConv2dFloat VERSION 1.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(GPGPU_GLES REQUIRED)

add_executable(GPGPUGLESChainConv2dFloat
        ${CMAKE_CURRENT_LIST_DIR}/chain_conv2d_float.c)

target_link_libraries(GPGPUGLESChainConv2dFloat PRIVATE GPGPU_GLES::GPGPU_GLES)
