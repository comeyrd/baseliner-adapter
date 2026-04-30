if(ADAPTERS_HAS_CUDA)
  add_subdirectory(cuda)
endif()

if(ADAPTERS_HAS_HIP)
  add_subdirectory(hip)
endif()


