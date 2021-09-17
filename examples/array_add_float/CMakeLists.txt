cmake_minimum_required(VERSION 3.7)
project(GPGPUGLESArrayAddFloat VERSION 1.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(GPGPU_GLES REQUIRED)

add_executable(GPGPUGLESArrayAddFloat
        ${CMAKE_CURRENT_LIST_DIR}/array_add_float.c)

target_link_libraries(GPGPUGLESArrayAddFloat PRIVATE GPGPU_GLES::GPGPU_GLES)
