set(GPGPU_GLES_EXAMPLES
    array_add_float
    fir_conv_float
    array_add_fixed16
    mult_mat_int
    # chain API
    chain_simple_float
    chain_conv2d_float
)

foreach(EXAMPLE ${GPGPU_GLES_EXAMPLES})
        message("==> CMake generation for ${EXAMPLE}...")
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/${EXAMPLE})
endforeach()
