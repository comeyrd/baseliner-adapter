#ifndef ADAPTER_MINIMALREGISTER_HPP
#define ADAPTER_MINIMALREGISTER_HPP

#include <adapters/registrars/MinimalRegistrar.hpp>

/// ---------------------------------------------------------------------------
/// Internal implementation macros
/// ---------------------------------------------------------------------------

#define MINIMAL_REGISTER_WORKLOAD_IMPL(WorkloadType)                                                                   \
  namespace {                                                                                                          \
    static const Adapters::RegisterWorkload<WorkloadType> _MINIMAL_reg_##WorkloadType{};                               \
  }

/// ---------------------------------------------------------------------------
/// Public macros
/// ---------------------------------------------------------------------------

#define MINIMAL_REGISTER_WORKLOAD(Workload) MINIMAL_REGISTER_WORKLOAD_IMPL(Workload)

#define MINIMAL_REGISTER_WORKLOAD_NAME(Workload, name) MINIMAL_REGISTER_WORKLOAD_IMPL(Workload)

#endif // ADAPTER_MINIMALREGISTER_HPP