#pragma once
#include <baseliner/core/Workload.hpp>

namespace Adapters {
  template <class WorkloadT>
  class BaselinerRegistrar {
  public:
    explicit BaselinerRegistrar(const std::string &name) {
      Baseliner::WorkloadRegistrar<WorkloadT> registrar(name);
    }
  };
} // namespace Adapters