add_executable(matmul_baseliner ExampleWorkload.hip)
target_link_libraries(matmul_baseliner PRIVATE baseliner_adapters::baseliner)
target_include_directories(matmul_baseliner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(matmul_primbench ExampleWorkload.hip)
target_link_libraries(matmul_primbench PRIVATE baseliner_adapters::primbench)
target_include_directories(matmul_primbench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)