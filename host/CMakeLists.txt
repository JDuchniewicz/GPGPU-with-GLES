cmake_minimum_required(VERSION 3.5)
project(powervr_host VERSION 1.0.0 LANGUAGES CXX)

# specify the sources
set(SRCS
        ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
    )

# this is the host project using powerVR SDK for testing the GPGPU library
add_executable(${PROJECT_NAME}
                ${SRCS})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include/
    )

# add the PowerVR libary
find_library(%{PVR_SHELL}
                PVRShell
                ${CMAKE_CURRENT_LIST_DIR}/lib)

target_link_libraries(${PROJECT_NAME}
                        PRIVATE
                        ${PVR_SHELL}
                     )

# add GPGPU library?

