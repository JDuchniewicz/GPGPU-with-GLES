cmake_minimum_required(VERSION 3.13)
project(GPGPUGLESBasic)

find_package(GPGPU_GLES REQUIRED MODULE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(SRCS
        ${CMAKE_CURRENT_LIST_DIR}/basic.c)

add_executable(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${GPGPU_GLES_INCLUDE_DIRECTORIES})

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        GPGPU_GLES)
