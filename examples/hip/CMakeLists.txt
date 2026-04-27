adapter_add_workload(
    NAME   matmul
    SOURCES ExampleWorkload.hip
)
target_include_directories(matmul_baseliner  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(matmul_primbench    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)