# FindEGL.cmake
#
# Finds the EGL library and its dependencies
#
# If EGL_INCLUDE_DIR is not set, sets it to the default directory on the Imagination BBB image:
#       /opt/Native_SDK/include
#
# This will define the following variables
#       EGL_FOUND
#  and the following imported targets
#       EGL::EGL
#
# Author: Jakub Duchniewicz - j.duchniewicz@gmail.com

find_library(EGL_LIBRARY NAMES
    EGL
)

if (NOT DEFINED EGL_INCLUDE_DIR)
    set(EGL_INCLUDE_DIR "/opt/Native_SDK/include")
endif()

find_package(PackageHandleStandardArgs REQUIRED)
find_package_handle_standard_args(EGL
    DEFAULT_MSG
    EGL_LIBRARY)

if (NOT TARGET EGL::EGL)
    add_library(EGL::EGL UNKNOWN IMPORTED)
    set_target_properties(EGL::EGL PROPERTIES
        IMPORTED_LOCATION "${EGL_LIBRARY}")
    set_target_properties(EGL::EGL PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${EGL_INCLUDE_DIR}")
endif()

mark_as_advanced(EGL_LIBRARY)

