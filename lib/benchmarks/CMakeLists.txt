set(GPGPU_GLES_BENCHMARKS
    first
)

foreach(BENCHMARK ${GPGPU_GLES_BENCHMARKS})
        message("==> CMake generation for ${BENCHMARK}...")
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/${BENCHMARK})
endforeach()
