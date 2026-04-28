#ifndef ADAPTER_NVBENCHREGISTER_HPP
#define ADAPTER_NVBENCHREGISTER_HPP

#include <adapters/registrars/NVBenchRegistrar.hpp>
#include <baseliner/core/Kernel.hpp>
#include <nvbench/nvbench.cuh>

/// ---------------------------------------------------------------------------
/// Internal implementation macros
/// ---------------------------------------------------------------------------

#define NVBENCH_REGISTER_WORKLOAD_IMPL(W)                                                                              \
  static void _##W##_nvbench_fn(nvbench::state &state) {                                                               \
    Adapters::NvbenchRegistrar<W>::run(state);                                                                         \
  }                                                                                                                    \
  NVBENCH_BENCH(_##W##_nvbench_fn);

#define NVBENCH_REGISTER_KERNEL_IMPL(K)                                                                                \
  static void _##K##_nvbench_fn(nvbench::state &state) {                                                               \
    Adapters::NvbenchRegistrar<Baseliner::KernelWorkload<K>>::run(state);                                              \
  }                                                                                                                    \
  NVBENCH_BENCH(_##K##_nvbench_fn);

#define NVBENCH_REGISTER_WORKLOAD(Workload) NVBENCH_REGISTER_WORKLOAD_IMPL(Workload)

#define NVBENCH_REGISTER_WORKLOAD_NAME(Workload, name) NVBENCH_REGISTER_WORKLOAD_IMPL(Workload)

#define NVBENCH_REGISTER_KERNEL(Kernel) NVBENCH_REGISTER_KERNEL_IMPL(Kernel)

#endif // ADAPTER_NVBENCHREGISTER_HPP