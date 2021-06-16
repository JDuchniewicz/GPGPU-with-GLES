# FindOpenGLES2.cmake
#
# Finds the OpenGLES2 library and its dependencies
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

find_package(PackageHandleStandardArgs REQUIRED)
find_package_handle_standard_args(OpenGLES2
    DEFAULT_MSG
    OPENGLES2_LIBRARY)

if (NOT TARGET OpenGLES2::OpenGLES2)
    add_library(OpenGLES2::OpenGLES2 INTERFACE IMPORTED)
    #set_target_properties(OpenGLES2::OpenGLES2 PROPERTIES
    #    IMPORTED_LOCATION ${OPENGLES2_LIBRARY}) # is it needed though?
endif()

mark_as_advanced(OPENGLES2_LIBRARY)

