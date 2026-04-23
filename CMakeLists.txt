cmake_minimum_required(VERSION 3.15)
project(baseliner-adapters VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ADAPTERS_BUILD_EXAMPLES "Build adapter examples" OFF)

include(CheckLanguage)

check_language(CUDA)
if(CMAKE_CUDA_COMPILER)
    enable_language(CUDA)
    set(CMAKE_CUDA_RUNTIME_LIBRARY Shared)
    set(ADAPTERS_HAS_CUDA ON)
endif()

check_language(HIP)
if(CMAKE_HIP_COMPILER)
    enable_language(HIP)
    set(ADAPTERS_HAS_HIP ON)
endif()

include(FetchContent)

# ── Fetch Baseliner ────────────────────────────────────────────────
FetchContent_Declare(
    baseliner
    GIT_REPOSITORY https://github.com/comeyrd/gpu-kernel-baseliner.git
    GIT_TAG        develop
)
FetchContent_MakeAvailable(baseliner)
include(${baseliner_SOURCE_DIR}/cmake/utils.cmake)

# ── Fetch NVBench (CUDA only) ──────────────────────────────────────
if(ADAPTERS_HAS_CUDA)
    FetchContent_Declare(
        nvbench
        GIT_REPOSITORY https://github.com/NVIDIA/nvbench.git
        GIT_TAG        main
    )
    FetchContent_MakeAvailable(nvbench)
    set(ADAPTERS_HAS_NVBENCH ON)
endif()

# ── Fetch Primbench header (HIP only) ─────────────────────────────
if(ADAPTERS_HAS_HIP)
    set(PRIMBENCH_INCLUDE_DIR "${CMAKE_BINARY_DIR}/_deps/primbench/include")
    file(MAKE_DIRECTORY ${PRIMBENCH_INCLUDE_DIR})

    FetchContent_Declare(
        primbench_header
        URL      https://raw.githubusercontent.com/ROCm/rocm-libraries/refs/heads/develop/shared/primbench/primbench.hpp
        DOWNLOAD_NO_EXTRACT TRUE
    )
    FetchContent_MakeAvailable(primbench_header)

    # FetchContent puts it in the content dir, copy to our include dir
    file(COPY ${primbench_header_SOURCE_DIR}/primbench.hpp
         DESTINATION ${PRIMBENCH_INCLUDE_DIR})

    set(ADAPTERS_HAS_PRIMBENCH ON)
endif()

# ── Interface target ───────────────────────────────────────────────
add_library(baseliner_adapters INTERFACE)
add_library(baseliner_adapters::baseliner_adapters ALIAS baseliner_adapters)

target_include_directories(baseliner_adapters INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(ADAPTERS_HAS_PRIMBENCH)
    target_include_directories(baseliner_adapters INTERFACE
        $<BUILD_INTERFACE:${PRIMBENCH_INCLUDE_DIR}>
    )
endif()

target_link_libraries(baseliner_adapters INTERFACE baseliner::baseliner)

# ── Expose cmake helpers ───────────────────────────────────────────
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/adapters.cmake)

if(ADAPTERS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()