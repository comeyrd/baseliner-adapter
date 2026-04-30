#pragma once

#include <baseliner/core/Workload.hpp>

#include <functional>
#include <memory>
#include <vector>

namespace Adapters {

  inline auto work_size_omap(int work_size) -> Baseliner::OptionsMap {
    std::string str_wz = std::to_string(work_size);
    auto option = Baseliner::Option{{}, str_wz};
    Baseliner::OptionsMap omap = {{"Workload", {{"work_size", option}}}};
    return omap;
  } // namespace Adapters
  class IWorkloadBridge {
  public:
    virtual ~IWorkloadBridge() = default;                  // Essential for safe polymorphic destruction
    virtual void run(int work_size, uint repetitions) = 0; // Type-erased execution
    virtual auto timed_run(int work_size, uint repetitions)
        -> std::vector<Baseliner::float_milliseconds> = 0; // Type-erased execution
    virtual auto name() -> std::string = 0;
  };

  template <typename BackendT>
  using WorkloadFactory = std::function<std::unique_ptr<Baseliner::IWorkload<BackendT>>()>;

  template <typename BackendT>
  struct WorkloadBridge : public IWorkloadBridge {
    explicit WorkloadBridge(WorkloadFactory<BackendT> workload)
        : m_workload(workload()) {
    }
    auto name() -> std::string override {
      return m_workload->algo() + m_workload->specialization();
    }

    void run(int work_size, uint repetitions) override {
      auto stream = BackendT::instance()->create_stream();
      m_workload->apply_options(work_size_omap(work_size));
      m_workload->setup_host();
      m_workload->setup_device(*stream);
      for (uint j = 0; j < repetitions; j++) {
        m_workload->reset_device(*stream);
        m_workload->run(*stream);
      }
      m_workload->fetch_results(*stream);
      m_workload->free();
    }

    auto timed_run(int work_size, uint repetitions) -> std::vector<Baseliner::float_milliseconds> override {
      m_workload->apply_options(work_size_omap(work_size));
      auto stream = BackendT::instance()->create_stream();
      m_workload->set_timer(std::make_shared<Baseliner::Hardware::GpuTimer<BackendT>>());
      m_workload->setup_host();
      m_workload->setup_device(*stream);

      m_workload->init_batch(*stream, repetitions, false);
      for (uint j = 0; j < repetitions; j++) {
        m_workload->reset_device(*stream);
        m_workload->timed_batch_run(*stream);
      }
      auto exec_time = m_workload->timed_run_elapsed_batch();
      m_workload->fetch_results(*stream);
      m_workload->free();
      return exec_time;
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