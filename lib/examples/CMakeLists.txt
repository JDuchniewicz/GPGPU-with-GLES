set(GPGPU_GLES_EXAMPLES
    basic
    )_

foreach(EXAMPLE ${GPGPU_GLES_EXAMPLES}
        message("==> CMake generation for ${EXAMPLE}...")
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/${EXAMPLE})
endforeach()
