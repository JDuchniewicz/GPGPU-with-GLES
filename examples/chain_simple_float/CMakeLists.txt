cmake_minimum_required(VERSION 3.7)
project(GPGPUGLESChainSimpleFloat VERSION 1.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(GPGPU_GLES REQUIRED)

add_executable(GPGPUGLESChainSimpleFloat
        ${CMAKE_CURRENT_LIST_DIR}/chain_simple_float.c)

target_link_libraries(GPGPUGLESChainSimpleFloat PRIVATE GPGPU_GLES::GPGPU_GLES)
