#ifndef ADAPTER_PRIMBENCHREGISTER_HPP
#define ADAPTER_PRIMBENCHREGISTER_HPP

#include <adapters/registrars/PrimbenchRegistrar.hpp>

/// ---------------------------------------------------------------------------
/// Internal implementation macros
/// ---------------------------------------------------------------------------

#define PRIMBENCH_REGISTER_WORKLOAD_IMPL(WorkloadType)                                                                 \
  namespace {                                                                                                          \
    static const Adapters::RegisterWorkload<WorkloadType> _primbench_reg_##WorkloadType{};                             \
  }

/// ---------------------------------------------------------------------------
/// Public macros
/// ---------------------------------------------------------------------------

#define PRIMBENCH_REGISTER_WORKLOAD(Workload) PRIMBENCH_REGISTER_WORKLOAD_IMPL(Workload)

#define PRIMBENCH_REGISTER_WORKLOAD_NAME(Workload, name) PRIMBENCH_REGISTER_WORKLOAD_IMPL(Workload)

#endif // ADAPTER_PRIMBENCHREGISTER_HPP