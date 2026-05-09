// bench_main.cpp
#include <adapters/PrimBenchRegister.hpp>
#include <memory>
#include <primbench.hpp>
#include <vector>

namespace {

  struct WorkloadBridge : public primbench::benchmark_interface {
    using Workload = Adapters::ActiveWorkload;

    WorkloadBridge(std::unique_ptr<Workload> w, int work_size)
        : m_workload(std::move(w)),
          m_worksize(work_size) {
    }

    primbench::json meta() const override {
      return primbench::json{}
          .add("algo", m_workload->algo())
          .add("name", m_workload->specialization())
          .add("work_size", m_worksize);
    }

    void run(primbench::state &state) override {
      m_workload->set_worksize(m_worksize);
      m_workload->setup_host();
      m_workload->setup_device(state.stream);

      auto flops = m_workload->number_of_floating_point_operations();
      if (flops.has_value() && flops.value() > 0)
        state.set_items(flops.value());
      else
        state.set_items(state.size);

      if (auto bytes = m_workload->number_of_bytes(); bytes.has_value())
        state.add_writes<uint8_t>(bytes.value());

      state.run_before_every_iteration([&] { m_workload->reset_device(state.stream); });
      state.run([&] { m_workload->run(state.stream); });

      m_workload->fetch_results(state.stream);
      m_workload->free();
    }

  private:
    std::unique_ptr<Workload> m_workload;
    int m_worksize;
  };

} // namespace

int main(int argc, char *argv[]) {
#ifdef BASELINER_BACKEND_CUDA
  primbench::executor exec(argc, argv, primbench::settings{}, primbench::flags::sync);
#endif
#ifdef BASELINER_BACKEND_HIP
  primbench::executor exec(argc, argv);
#endif
  const std::vector<int> work_sizes = {1, 2, 4, 8, 16, 32, 64, 128, 256};

  auto &storage = Adapters::PrimBenchStorage<Adapters::ActiveBackend>::instance();
  for (const auto &factory : storage.factories()) {
    for (int w : work_sizes) {
      exec.queue<WorkloadBridge>(factory(), w);
    }
  }
  exec.run();
  return 0;
}