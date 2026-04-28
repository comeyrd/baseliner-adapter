#ifndef ADAPTER_PRIMBENCHREGISTER_HPP
#define ADAPTER_PRIMBENCHREGISTER_HPP

#include <adapters/registrars/PrimbenchRegistrar.hpp>
#include <baseliner/core/Kernel.hpp>

/// ---------------------------------------------------------------------------
/// Internal implementation macros
/// ---------------------------------------------------------------------------

#define PRIMBENCH_REGISTER_WORKLOAD_IMPL(WorkloadType)                                                                 \
  namespace {                                                                                                          \
    static const Adapters::RegisterWorkload<WorkloadType> _primbench_reg_##WorkloadType{};                             \
  }

#define PRIMBENCH_REGISTER_KERNEL_IMPL(KernelType)                                                                     \
  namespace {                                                                                                          \
    static const Adapters::RegisterWorkload<Baseliner::KernelWorkload<KernelType>> _primbench_reg_##KernelType{};      \
  }

/// ---------------------------------------------------------------------------
/// Public macros
/// ---------------------------------------------------------------------------

#define PRIMBENCH_REGISTER_WORKLOAD(Workload) PRIMBENCH_REGISTER_WORKLOAD_IMPL(Workload)

#define PRIMBENCH_REGISTER_WORKLOAD_NAME(Workload, name) PRIMBENCH_REGISTER_WORKLOAD_IMPL(Workload)

#define PRIMBENCH_REGISTER_KERNEL(Kernel) PRIMBENCH_REGISTER_KERNEL_IMPL(Kernel)

#endif // ADAPTER_PRIMBENCHREGISTER_HPP