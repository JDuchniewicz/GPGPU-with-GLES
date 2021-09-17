cmake_minimum_required(VERSION 3.7)
project(GPGPU_GLES VERSION 1.0.0 LANGUAGES C)

# Add the custom modules to CMake search path
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

##################################
# Find system dependencies

#find_package(OpenGL REQUIRED COMPONENTS EGL)
find_package(EGL REQUIRED)
find_package(OpenGLES2 REQUIRED)

##################################
# Create target and set properties

add_library(GPGPU_GLES STATIC
    ${CMAKE_CURRENT_LIST_DIR}/src/gpgpu_gles.c
    ${CMAKE_CURRENT_LIST_DIR}/src/gpgpu_int.c
    ${CMAKE_CURRENT_LIST_DIR}/src/debug.c
    ${CMAKE_CURRENT_LIST_DIR}/src/shaders.c
)

if(CMAKE_HOST_SYSTEM_VERSION MATCHES ".*-ti-.*")
	target_compile_options(GPGPU_GLES PRIVATE -O3 -DBEAGLE)
else()
	target_compile_options(GPGPU_GLES PRIVATE -O3)
endif()

# add alias to use it as GPGPU_GLES::GPGPU_GLES
add_library(GPGPU_GLES::GPGPU_GLES ALIAS GPGPU_GLES)

target_include_directories(GPGPU_GLES
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${EGL_INCLUDE_DIR}> # need it only once
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/include
)

target_link_libraries(GPGPU_GLES
    PRIVATE
    EGL::EGL OpenGLES2::OpenGLES2 gbm
)
###################################
# Installation instructions

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/GPGPU_GLES)

install(TARGETS GPGPU_GLES
    EXPORT GPGPU_GLES_TARGETS
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export the targets to a script (This script is then used by CMakeConfig file)
install(EXPORT GPGPU_GLES_TARGETS
    FILE
        GPGPU_GLESTargets.cmake
    NAMESPACE
        GPGPU_GLES::
    DESTINATION
        ${INSTALL_CONFIGDIR}
)

# Create a ConfigVersion.cmake
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/GPGPU_GLESConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/GPGPU_GLESConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/GPGPU_GLESConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

# Install the config, configversion and custom find modules
install(FILES
    ${CMAKE_CURRENT_LIST_DIR}/cmake/FindOpenGLES2.cmake
    ${CMAKE_CURRENT_LIST_DIR}/cmake/FindEGL.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/GPGPU_GLESConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/GPGPU_GLESConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

##################################
# Exporting from the build tree
configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/FindOpenGLES2.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/FindOpenGLES2.cmake
    COPYONLY)
configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/FindEGL.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/FindEGL.cmake
    COPYONLY)

export(EXPORT GPGPU_GLES_TARGETS
    FILE ${CMAKE_BINARY_DIR}/GPGPU_GLESTargets.cmake
    NAMESPACE GPGPU_GLES::)

export(PACKAGE GPGPU_GLES)

###############################
# build the examples one by one
add_subdirectory(examples)
add_subdirectory(benchmarks)
