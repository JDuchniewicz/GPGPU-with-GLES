cmake_minimum_required(VERSION 3.5)
project(gpgpu_gles VERSION 1.0.0 LANGUAGES C)

# includes
target_include_directories(${PROJECT_NAME}
                            PUBLIC
                                $<INSTALL_INTERFACE:include>
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOUREC_DIR}/include>
                          )

add_library(${PROJECT_NAME}
                ${SRCS})

# this is the host project using powerVR SDK for testing the GPGPU library
