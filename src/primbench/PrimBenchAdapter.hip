#include <adapters/PrimBenchRegister.hpp>
#include <primbench.hpp>

#include <memory>

namespace {

  struct WorkloadBridge : public primbench::benchmark_interface {

    explicit WorkloadBridge(std::unique_ptr<Adapters::HipWorkload> workload)
        : m_workload(std::move(workload)) {
    }

    primbench::json meta() const override {
      return primbench::json{}.add("algo", m_workload->algo()).add("name", m_workload->specialization());
    }

    void run(primbench::state &state) override {
      // primbench owns the stream; wrap it as a non-owning shared_ptr
      // so Baseliner's interface is satisfied without a double-free.
      auto stream = std::shared_ptr<hipStream_t>(&const_cast<hipStream_t &>(state.stream),
                                                 [](hipStream_t *) { /* no-op: primbench owns the stream */ });

      m_workload->setup_host();
      m_workload->setup_device(stream);

      // Tell primbench how many items/bytes this workload processes
      // so it can compute items/sec and bytes/sec.
      auto flops = m_workload->number_of_floating_point_operations();
      if (flops.has_value() && flops.value() > 0) {
        state.set_items(flops.value());
      } else {
        state.set_items(state.size);
      }

      auto bytes = m_workload->number_of_bytes();
      if (bytes.has_value()) {
        state.add_writes<uint8_t>(bytes.value());
      }

      state.run_before_every_iteration([&]() { m_workload->reset_device(stream); });
      state.run([&]() { m_workload->run(stream); });

      m_workload->fetch_results(stream);
      m_workload->free();
    }

  private:
    std::unique_ptr<Adapters::HipWorkload> m_workload;
  };

} // anonymous namespace

int main(int argc, char *argv[]) {
  primbench::executor exec(argc, argv);

  for (auto &workload : Adapters::PrimBenchStorage::instance().take_workloads()) {
    exec.queue<WorkloadBridge>(std::move(workload));
  }

  exec.run();
  return 0;
}