# FindEGL.cmake
#
# Finds the EGL library and its dependencies
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

find_package(PackageHandleStandardArgs REQUIRED)
find_package_handle_standard_args(EGL
    DEFAULT_MSG
    EGL_LIBRARY)

if (NOT TARGET EGL::EGL)
    add_library(EGL::EGL INTERFACE IMPORTED)
endif()

mark_as_advanced(EGL_LIBRARY)

