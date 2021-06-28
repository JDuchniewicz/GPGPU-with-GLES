cmake_minimum_required(VERSION 3.13)
project(GPGPUGLESBasic VERSION 1.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(GPGPU_GLES REQUIRED)

add_executable(GPGPUGLESBasic
        ${CMAKE_CURRENT_LIST_DIR}/basic.c)

target_link_libraries(GPGPUGLESBasic PRIVATE GPGPU_GLES::GPGPU_GLES)
