cmake_minimum_required(VERSION 3.15)
project(baseliner-adapters VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    set(ROCM_PATH "/opt/rocm" CACHE PATH "Path to ROCm installation")
    find_path(AMDSMI_INCLUDE_DIR NAMES amd_smi/amdsmi.h PATHS ${ROCM_PATH}/include)
    find_library(AMDSMI_LIBRARY NAMES amd_smi PATHS ${ROCM_PATH}/lib)

    if(AMDSMI_INCLUDE_DIR AND AMDSMI_LIBRARY)
        message(STATUS "Found AMD SMI: ${AMDSMI_LIBRARY}")  
        set(BASELINER_HAS_AMDSMI ON)
        add_library(ADAPTER_AMD::amdsmi UNKNOWN IMPORTED)
        set_target_properties(ADAPTER_AMD::amdsmi PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${AMDSMI_INCLUDE_DIR}"
            IMPORTED_LOCATION "${AMDSMI_LIBRARY}"
        )
    endif()
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

    file(COPY ${primbench_header_SOURCE_DIR}/primbench.hpp
         DESTINATION ${PRIMBENCH_INCLUDE_DIR})

    set(ADAPTERS_HAS_PRIMBENCH ON)
endif()

# ── Interface target ───────────────────────────────────────────────
if(ADAPTERS_HAS_CUDA OR ADAPTERS_HAS_HIP)
    add_library(baseliner_adapters_baseliner INTERFACE)
    add_library(baseliner_adapters::baseliner ALIAS baseliner_adapters_baseliner)
    target_include_directories(baseliner_adapters_baseliner INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    target_compile_definitions(baseliner_adapters_baseliner INTERFACE ADAPTER_BACKEND_BASELINER)
    target_link_libraries(baseliner_adapters_baseliner INTERFACE baseliner::baseliner)
endif()

if(ADAPTERS_HAS_NVBENCH AND ADAPTERS_HAS_CUDA)
    add_library(baseliner_adapters_nvbench INTERFACE)
    add_library(baseliner_adapters::nvbench ALIAS baseliner_adapters_nvbench)
    target_include_directories(baseliner_adapters_nvbench INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    target_compile_definitions(baseliner_adapters_nvbench INTERFACE ADAPTER_BACKEND_NVBENCH)
    target_link_libraries(baseliner_adapters_nvbench INTERFACE baseliner::specs nvbench::main)
endif()

if(ADAPTERS_HAS_PRIMBENCH AND ADAPTERS_HAS_HIP)
    add_library(baseliner_adapters_primbench INTERFACE)
    add_library(baseliner_adapters::primbench ALIAS baseliner_adapters_primbench)
    target_include_directories(baseliner_adapters_primbench INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${PRIMBENCH_INCLUDE_DIR}>
    )
    target_compile_definitions(baseliner_adapters_primbench INTERFACE ADAPTER_BACKEND_PRIMBENCH)
    target_link_libraries(baseliner_adapters_primbench INTERFACE baseliner::specs)
    if(ADAPTERS_HAS_AMDSMI)
        target_link_libraries(baseliner_adapters_primbench INTERFACE ADAPTER_AMD::amdsmi)
    else()
        target_compile_definitions(baseliner_adapters_primbench INTERFACE PRIMBENCH_NO_MONITORING)
    endif()
endif()

if(ADAPTERS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
