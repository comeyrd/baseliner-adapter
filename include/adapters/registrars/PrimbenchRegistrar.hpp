#pragma once
#include <baseliner/core/Workload.hpp>
#include <functional>
#include <memory>
#include <vector>

namespace Adapters {

  template <class Backend>
  class PrimBenchStorage {
  public:
    using Workload = Baseliner::IWorkload<Backend>;
    using Factory = std::function<std::unique_ptr<Workload>()>;

    static PrimBenchStorage &instance() {
      static PrimBenchStorage s;
      return s;
    }

    void register_workload(Factory f) {
      m_factories.push_back(std::move(f));
    }
    const std::vector<Factory> &factories() const {
      return m_factories;
    }

  private:
    PrimBenchStorage() = default;
    std::vector<Factory> m_factories;
  };

  template <class WorkloadT, class Backend>
  struct RegisterWorkload {
    RegisterWorkload() {
      PrimBenchStorage<Backend>::instance().register_workload(
          []() -> std::unique_ptr<Baseliner::IWorkload<Backend>> { return std::make_unique<WorkloadT>(); });
    }
  };

} // namespace Adapters