cmake_minimum_required(VERSION 3.5)
project(gpgpu_gles VERSION 1.0.0 LANGUAGES C)


set(SRCS
        ${CMAKE_CURRENT_LIST_DIR}/src/gpgpu_gles.c
    )

add_library(${PROJECT_NAME}
                ${SRCS})

# includes
target_include_directories(${PROJECT_NAME}
                            PUBLIC
                                $<INSTALL_INTERFACE:include>
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                          )
# this is the host project using powerVR SDK for testing the GPGPU library
