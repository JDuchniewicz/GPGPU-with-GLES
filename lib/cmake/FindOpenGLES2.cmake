# FindOpenGLES2.cmake
#
# Finds the OpenGLES2 library and its dependencies
#
# If GLES2_INCLUDE_DIR is not set, sets it to the default directory on the Imagination BBB image:
#       /opt/Native_SDK/include
#
# This will define the following variables
#       OpenGLES2_FOUND
#  and the following imported targets
#       OpenGLES2::OpenGLES2
#
# Author: Jakub Duchniewicz - j.duchniewicz@gmail.com

find_library(OPENGLES2_LIBRARY NAMES
    GLESv2
)

if (NOT DEFINED GLES2_INCLUDE_DIR)
    set(GLES2_INCLUDE_DIR "/opt/Native_SDK/include")
endif()

find_package(PackageHandleStandardArgs REQUIRED)
find_package_handle_standard_args(OpenGLES2
    DEFAULT_MSG
    OPENGLES2_LIBRARY)

if (NOT TARGET OpenGLES2::OpenGLES2)
    add_library(OpenGLES2::OpenGLES2 UNKNOWN IMPORTED)
    set_target_properties(OpenGLES2::OpenGLES2 PROPERTIES
        IMPORTED_LOCATION "${OPENGLES2_LIBRARY}")
    set_target_properties(OpenGLES2::OpenGLES2 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${GLES2_INCLUDE_DIR}")
endif()

mark_as_advanced(OPENGLES2_LIBRARY)

