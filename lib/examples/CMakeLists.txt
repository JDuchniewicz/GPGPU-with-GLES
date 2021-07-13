set(GPGPU_GLES_EXAMPLES
    basic
    fir_conv_float
    array_add_fixed16_argb8888
)

foreach(EXAMPLE ${GPGPU_GLES_EXAMPLES})
        message("==> CMake generation for ${EXAMPLE}...")
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/${EXAMPLE})
endforeach()
