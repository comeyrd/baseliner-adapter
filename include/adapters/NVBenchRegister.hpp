#ifndef ADAPTER_NVBENCHREGISTER_HPP
#define ADAPTER_NVBENCHREGISTER_HPP

#include <adapters/registrars/NVBenchRegistrar.hpp>
#include <nvbench/nvbench.cuh>

/// ---------------------------------------------------------------------------
/// Internal implementation macros
/// ---------------------------------------------------------------------------

#define NVBENCH_REGISTER_WORKLOAD_IMPL(W)                                                                              \
  static void _##W##_nvbench_fn(nvbench::state &state) {                                                               \
    Adapters::NvbenchRegistrar<W>::run(state);                                                                         \
  }                                                                                                                    \
  NVBENCH_BENCH(_##W##_nvbench_fn)                                                                                     \
      .set_name(Adapters::nv_bench_benchmark_name<W>())                                                                \
      .add_int64_power_of_two_axis("work_size", nvbench::range(0, 8));

#define NVBENCH_REGISTER_WORKLOAD(Workload) NVBENCH_REGISTER_WORKLOAD_IMPL(Workload)

#define NVBENCH_REGISTER_WORKLOAD_NAME(Workload, name) NVBENCH_REGISTER_WORKLOAD_IMPL(Workload)

#endif // ADAPTER_NVBENCHREGISTER_HPP