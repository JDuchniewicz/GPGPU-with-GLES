cmake_minimum_required(VERSION 3.13)
project(benchark1 VERSION 1.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/benchmark)

find_package(GPGPU_GLES REQUIRED)

add_executable(benchmark1
        ${CMAKE_CURRENT_LIST_DIR}/benchmark1.c)

target_link_libraries(benchmark1 PRIVATE GPGPU_GLES::GPGPU_GLES)
