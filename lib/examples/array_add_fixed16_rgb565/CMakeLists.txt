cmake_minimum_required(VERSION 3.13)
project(GPGPUGLESArrayAddFixed16RGB565 VERSION 1.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(GPGPU_GLES REQUIRED)

add_executable(GPGPUGLESArrayAddFixed16RGB565
        ${CMAKE_CURRENT_LIST_DIR}/array_add_fixed16_rgb565.c)

target_link_libraries(GPGPUGLESArrayAddFixed16RGB565 PRIVATE GPGPU_GLES::GPGPU_GLES)
