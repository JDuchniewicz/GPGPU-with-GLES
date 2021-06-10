cmake_minimum_required(VERSION 3.5)
project(powervr_host VERSION 1.0.0 LANGUAGES CXX)

# specify the sources

# add the PowerVR libary
target_link_libraries(${PROJECT_NAME}
                        PUBLIC
                            PVRShell # need to add it to the search path
                     )

# add GPGPU library?

add_executable(${PROJECT_NAME}
                ${SRCS})
# this is the host project using powerVR SDK for testing the GPGPU library
