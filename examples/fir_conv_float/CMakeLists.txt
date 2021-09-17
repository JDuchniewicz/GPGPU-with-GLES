cmake_minimum_required(VERSION 3.7)
project(GPGPUGLESFIRConvFloat VERSION 1.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(GPGPU_GLES REQUIRED)

add_executable(GPGPUGLESFIRConvFloat
        ${CMAKE_CURRENT_LIST_DIR}/fir_conv_float.c)

target_link_libraries(GPGPUGLESFIRConvFloat PRIVATE GPGPU_GLES::GPGPU_GLES)
