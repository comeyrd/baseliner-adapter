add_executable(matmul_baseliner ExampleWorkload.cu)
target_link_libraries(matmul_baseliner PRIVATE baseliner_adapters::baseliner)
target_include_directories(matmul_baseliner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(matmul_nvbench ExampleWorkload.cu)
target_link_libraries(matmul_nvbench PRIVATE baseliner_adapters::nvbench nvbench::main)
target_include_directories(matmul_nvbench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)