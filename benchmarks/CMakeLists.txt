cmake_minimum_required(VERSION 3.7)
project(benchark VERSION 1.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/benchmark)

find_package(GPGPU_GLES REQUIRED)

add_executable(benchmark
        ${CMAKE_CURRENT_LIST_DIR}/benchmark.c
        ${CMAKE_CURRENT_LIST_DIR}/computations.c)

target_compile_options(benchmark PRIVATE -O3)

target_link_libraries(benchmark PRIVATE GPGPU_GLES::GPGPU_GLES)
