#pragma once

#include <baseliner/core/Workload.hpp>
#include <baseliner/core/hardware/hip/HipBackend.hpp>

#include <functional>
#include <memory>
#include <vector>

namespace Adapters {

  using HipWorkload = Baseliner::IWorkload<Baseliner::Hardware::HipBackend>;
  using HipWorkloadFactory = std::function<std::unique_ptr<HipWorkload>()>;

  class PrimBenchStorage {
  public:
    static auto instance() -> PrimBenchStorage & {
      static PrimBenchStorage storage{};
      return storage;
    }

    void register_workload(HipWorkloadFactory factory) {
      m_factories.push_back(std::move(factory));
    }

    auto take_workloads() -> std::vector<std::unique_ptr<HipWorkload>> {
      std::vector<std::unique_ptr<HipWorkload>> workloads;
      workloads.reserve(m_factories.size());
      for (auto &factory : m_factories) {
        workloads.push_back(factory());
      }
      return workloads;
    }

  private:
    PrimBenchStorage() = default;
    std::vector<HipWorkloadFactory> m_factories;
  };

  template <typename WorkloadT>
  struct RegisterWorkload {
    RegisterWorkload() {
      PrimBenchStorage::instance().register_workload(
          []() -> std::unique_ptr<HipWorkload> { return std::make_unique<WorkloadT>(); });
    }
  };

} // namespace Adapters