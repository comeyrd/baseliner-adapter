add_library(matmul INTERFACE)
target_include_directories(matmul INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(MatMulWorkload)

add_executable(matmul_baseliner)
target_link_libraries(matmul_baseliner PUBLIC matmul baseliner_adapters::baseliner)
target_include_directories(matmul_baseliner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(matmul_minimal)
target_link_libraries(matmul_minimal PUBLIC matmul baseliner_adapters::minimal)
target_include_directories(matmul_minimal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)


if(ADAPTERS_HAS_CUDA)
    add_executable(matmul_nvbench)
    target_link_libraries(matmul_nvbench PUBLIC matmul baseliner_adapters::nvbench)
    target_include_directories(matmul_nvbench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
endif()

if(ADAPTERS_HAS_HIP)
    add_executable(matmul_primbench)
    target_link_libraries(matmul_primbench PRIVATE matmul baseliner_adapters::primbench )
    target_include_directories(matmul_primbench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
endif()