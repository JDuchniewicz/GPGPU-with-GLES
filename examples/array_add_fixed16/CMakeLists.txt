cmake_minimum_required(VERSION 3.7)
project(GPGPUGLESArrayAddFixed16 VERSION 1.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(GPGPU_GLES REQUIRED)

add_executable(GPGPUGLESArrayAddFixed16
        ${CMAKE_CURRENT_LIST_DIR}/array_add_fixed16.c)

target_link_libraries(GPGPUGLESArrayAddFixed16 PRIVATE GPGPU_GLES::GPGPU_GLES)
