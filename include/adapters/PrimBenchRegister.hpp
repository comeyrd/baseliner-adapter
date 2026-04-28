#ifndef ADAPTER_PRIMBENCHREGISTER_HPP
#define ADAPTER_PRIMBENCHREGISTER_HPP

#include <adapters/registrars/PrimbenchRegistrar.hpp>
#include <baseliner/core/Kernel.hpp>

/// ---------------------------------------------------------------------------
/// Internal implementation macros
/// ---------------------------------------------------------------------------

#define PRIMBENCH_REGISTER_WORKLOAD_IMPL(WorkloadType, UniqueSuffix)                                                   \
  namespace {                                                                                                          \
    static const Adapters::RegisterWorkload<WorkloadType> _primbench_reg_##UniqueSuffix{};                             \
  }

#define PRIMBENCH_REGISTER_KERNEL_IMPL(KernelType, UniqueSuffix)                                                       \
  namespace {                                                                                                          \
    static const Adapters::RegisterWorkload<Baseliner::KernelWorkload<KernelType>> _primbench_reg_##UniqueSuffix{};    \
  }

/// ---------------------------------------------------------------------------
/// Public macros
/// ---------------------------------------------------------------------------

#define PRIMBENCH_REGISTER_WORKLOAD(Workload) PRIMBENCH_REGISTER_WORKLOAD_IMPL(Workload, __COUNTER__)

#define PRIMBENCH_REGISTER_WORKLOAD_NAME(Workload, name) PRIMBENCH_REGISTER_WORKLOAD_IMPL(Workload, __COUNTER__)

#define PRIMBENCH_REGISTER_KERNEL(Kernel) PRIMBENCH_REGISTER_KERNEL_IMPL(Kernel, __COUNTER__)

#endif // ADAPTER_PRIMBENCHREGISTER_HPP