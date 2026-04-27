include_guard(GLOBAL)

# adapter_add_workload(
#   NAME      my_bench
#   SOURCES   my_bench.cu
#   LIBRARIES somelib::somelib
# )
function(adapter_add_workload)
    cmake_parse_arguments(ARG "" "NAME" "SOURCES;LIBRARIES" ${ARGN})

    if(NOT ARG_NAME)
        message(FATAL_ERROR "adapter_add_workload: NAME is required")
    endif()
    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "adapter_add_workload: SOURCES is required")
    endif()

    # ── Baseliner target ───────────────────────────────────────────
    if(ADAPTERS_HAS_CUDA)
        add_executable(${ARG_NAME}_baseliner ${ARG_SOURCES})
        target_compile_definitions(${ARG_NAME}_baseliner PRIVATE ADAPTER_BACKEND_BASELINER)
        target_link_libraries(${ARG_NAME}_baseliner PRIVATE
            baseliner_adapters::baseliner_adapters
            baseliner::baseliner
            ${ARG_LIBRARIES}
        )
        set_target_properties(${ARG_NAME}_baseliner PROPERTIES
            CUDA_SEPARABLE_COMPILATION ON
        )
    elseif(ADAPTERS_HAS_HIP)
        add_executable(${ARG_NAME}_baseliner ${ARG_SOURCES})
        target_compile_definitions(${ARG_NAME}_baseliner PRIVATE ADAPTER_BACKEND_BASELINER)
        target_link_libraries(${ARG_NAME}_baseliner PRIVATE
            baseliner_adapters::baseliner_adapters
            baseliner::baseliner
            ${ARG_LIBRARIES}
        )
    endif()

    # ── NVBench target (CUDA only) ─────────────────────────────────
    if(ADAPTERS_HAS_NVBENCH AND ADAPTERS_HAS_CUDA)
        add_executable(${ARG_NAME}_nvbench ${ARG_SOURCES})
        target_compile_definitions(${ARG_NAME}_nvbench PRIVATE ADAPTER_BACKEND_NVBENCH)
        target_link_libraries(${ARG_NAME}_nvbench PRIVATE
            baseliner_adapters::baseliner_adapters
            baseliner::specs
            nvbench::main
            ${ARG_LIBRARIES}
        )
        set_target_properties(${ARG_NAME}_nvbench PROPERTIES
            CUDA_SEPARABLE_COMPILATION ON
        )
    endif()

    # ── Primbench target (HIP only) ────────────────────────────────
    if(ADAPTERS_HAS_PRIMBENCH AND ADAPTERS_HAS_HIP)
        add_executable(${ARG_NAME}_primbench ${ARG_SOURCES})
        target_compile_definitions(${ARG_NAME}_primbench PRIVATE ADAPTER_BACKEND_PRIMBENCH)
        target_link_libraries(${ARG_NAME}_primbench PRIVATE
            baseliner_adapters::baseliner_adapters
            baseliner::specs
            ${ARG_LIBRARIES}
        )
    endif()

endfunction()