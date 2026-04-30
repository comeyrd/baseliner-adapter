#pragma once

#include <baseliner/core/Workload.hpp>

#include <functional>
#include <memory>
#include <vector>

namespace Adapters {

  class IWorkloadBridge {
  public:
    virtual ~IWorkloadBridge() = default; // Essential for safe polymorphic destruction
    virtual void run() = 0;               // Type-erased execution
  };

  template <typename BackendT>
  using WorkloadFactory = std::function<std::unique_ptr<Baseliner::IWorkload<BackendT>>()>;

  template <typename BackendT>
  struct WorkloadBridge : public IWorkloadBridge {
    explicit WorkloadBridge(WorkloadFactory<BackendT> workload)
        : m_workload(workload()) {
    }

    // Override the run method
    void run() override {
      auto stream = BackendT::instance()->create_stream();
      m_workload->setup_host();
      m_workload->setup_device(*stream);
      for (int j = 0; j < 50; j++) {
        m_workload->reset_device(*stream);
        m_workload->run(*stream);
      }
      m_workload->fetch_results(*stream);
      m_workload->free();
    }

  private:
    std::unique_ptr<Baseliner::IWorkload<BackendT>> m_workload;
  };
  class GlobalWorkloadStorage {
  public:
    static auto instance() -> GlobalWorkloadStorage & {
      static GlobalWorkloadStorage storage{};
      return storage;
    }

    void register_workload(std::unique_ptr<IWorkloadBridge> bridge) {
      m_workloads.push_back(std::move(bridge));
    }

    auto get_workloads() const -> const std::vector<std::unique_ptr<IWorkloadBridge>> & {
      return m_workloads;
    }

  private:
    GlobalWorkloadStorage() = default;
    std::vector<std::unique_ptr<IWorkloadBridge>> m_workloads;
  };

  template <typename WorkloadT>
  struct RegisterWorkload {
    RegisterWorkload() {
      GlobalWorkloadStorage::instance().register_workload(std::make_unique<WorkloadBridge<typename WorkloadT::backend>>(
          []() -> std::unique_ptr<Baseliner::IWorkload<typename WorkloadT::backend>> {
            return std::make_unique<WorkloadT>();
          }));
    }
  };

} // namespace Adapters