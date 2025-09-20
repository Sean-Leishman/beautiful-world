cmake_minimum_required(VERSION 3.16)

# Set compilers before project()
set(CMAKE_C_COMPILER clang-21)
set(CMAKE_CXX_COMPILER clang++-21)

project(raucous_wart LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
message(STATUS "PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}")
message(STATUS "CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Base flags for all builds
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# Debug flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fno-omit-frame-pointer")

# Release flags with maximum optimizations
set(CMAKE_CXX_FLAGS_RELEASE
    "-O3 \
    -march=native \
    -mtune=native \
    -ffast-math \
    -fno-signed-zeros \
    -fno-trapping-math \
    -fassociative-math \
    -freciprocal-math \
    -ffinite-math-only \
    -funroll-loops \
    -ftree-vectorize \
    -fvectorize \
    -fslp-vectorize \
    -DNDEBUG"
)

# RelWithDebInfo - optimized but debuggable
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -march=native -DNDEBUG")

# MinSizeRel - optimize for size
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -march=native -DNDEBUG")

# Default to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "CXX Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX Flags Release: ${CMAKE_CXX_FLAGS_RELEASE}")

# Find OpenMP for Clang
# Clang requires explicit flags for OpenMP
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Try to find libomp
    find_library(OpenMP_LIBRARY
        NAMES omp gomp
        HINTS
            /usr/lib/llvm-21/lib
            /usr/lib/x86_64-linux-gnu
            /usr/local/lib
            /usr/lib
    )

    if(OpenMP_LIBRARY)
        message(STATUS "Found OpenMP library: ${OpenMP_LIBRARY}")
        set(OpenMP_CXX_FLAGS "-fopenmp")
        set(OpenMP_CXX_LIB_NAMES "omp")
        set(OpenMP_omp_LIBRARY ${OpenMP_LIBRARY})
    else()
        message(STATUS "OpenMP library not found, trying manual flags")
        set(OpenMP_CXX_FLAGS "-fopenmp")
    endif()

    # Add OpenMP flags to compilation
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fopenmp")

    # Create imported target if it doesn't exist
    if(NOT TARGET OpenMP::OpenMP_CXX)
        add_library(OpenMP::OpenMP_CXX INTERFACE IMPORTED)
        set_target_properties(OpenMP::OpenMP_CXX PROPERTIES
            INTERFACE_COMPILE_OPTIONS "-fopenmp"
            INTERFACE_LINK_LIBRARIES "-fopenmp"
        )
        if(OpenMP_LIBRARY)
            set_target_properties(OpenMP::OpenMP_CXX PROPERTIES
                INTERFACE_LINK_LIBRARIES "${OpenMP_LIBRARY}"
            )
        endif()
    endif()

    message(STATUS "OpenMP configured for Clang")
else()
    # For GCC and other compilers, use standard FindOpenMP
    find_package(OpenMP REQUIRED)
    if(OpenMP_CXX_FOUND)
        message(STATUS "OpenMP found: ${OpenMP_CXX_VERSION}")
    endif()
endif()

# Add library
add_subdirectory(src)

# Main executable
add_executable(main main.cpp)
target_compile_features(main PRIVATE cxx_std_20)
target_link_libraries(main PRIVATE raytracer_lib OpenMP::OpenMP_CXX)

# Enable LTO for Release builds if supported
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
if(ipo_supported)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        message(STATUS "LTO enabled for Release build")
        set_property(TARGET main PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
        set_property(TARGET raytracer_lib PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
else()
    message(STATUS "LTO not supported: ${ipo_error}")
endif()
