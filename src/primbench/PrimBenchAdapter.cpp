// bench_main.cpp
#include <adapters/PrimBenchRegister.hpp>
#include <memory>
#include <primbench.hpp>
#include <vector>
namespace {

  /// Attempts to instantiate, set up, run, and tear down a workload once with
  /// work_size=1. Returns true if every step succeeds without throwing or
  /// returning a runtime error; false otherwise.
  ///
  /// On the CUDA backend we also clear any sticky error state so the next
  /// workload isn't poisoned by a failure here.
  bool try_workload(const Adapters::PrimBenchStorage<Adapters::ActiveBackend>::Factory &factory) {
    using Workload = Adapters::ActiveWorkload;

    std::unique_ptr<Workload> workload;
    typename Adapters::ActiveBackend::stream_t stream{};

    try {
      workload = factory();
      if (!workload) {
        std::cerr << "[try_workload] factory returned null\n";
        return false;
      }

      workload->set_worksize(1);
      workload->setup_host();

#if defined(BASELINER_BACKEND_CUDA)
      if (auto err = cudaStreamCreate(&stream); err != cudaSuccess) {
        std::cerr << "[try_workload] cudaStreamCreate: " << cudaGetErrorString(err) << "\n";
        return false;
      }
#elif defined(BASELINER_BACKEND_HIP)
      if (auto err = hipStreamCreate(&stream); err != hipSuccess) {
        std::cerr << "[try_workload] hipStreamCreate: " << hipGetErrorString(err) << "\n";
        return false;
      }
#endif

      workload->setup_device(stream);
      workload->reset_device(stream);
      workload->run(stream);
      workload->fetch_results(stream); // this syncs the stream
      workload->free();

#if defined(BASELINER_BACKEND_CUDA)
      cudaStreamDestroy(stream);
      if (auto err = cudaGetLastError(); err != cudaSuccess) {
        std::cerr << "[try_workload] residual CUDA error: " << cudaGetErrorString(err) << "\n";
        return false;
      }
#elif defined(BASELINER_BACKEND_HIP)
      hipStreamDestroy(stream);
      if (auto err = hipGetLastError(); err != hipSuccess) {
        std::cerr << "[try_workload] residual HIP error: " << hipGetErrorString(err) << "\n";
        return false;
      }
#endif

      return true;

    } catch (const std::exception &e) {
      std::cerr << "[try_workload] exception: " << e.what() << "\n";
#if defined(BASELINER_BACKEND_CUDA)
      cudaGetLastError(); // clear sticky error
      if (stream)
        cudaStreamDestroy(stream);
#elif defined(BASELINER_BACKEND_HIP)
      hipGetLastError();
      if (stream)
        hipStreamDestroy(stream);
#endif
      return false;
    } catch (...) {
      std::cerr << "[try_workload] unknown exception\n";
#if defined(BASELINER_BACKEND_CUDA)
      cudaGetLastError();
      if (stream)
        cudaStreamDestroy(stream);
#elif defined(BASELINER_BACKEND_HIP)
      hipGetLastError();
      if (stream)
        hipStreamDestroy(stream);
#endif
      return false;
    }
  }

} // namespace
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
  primbench::executor exec(argc, argv);
  const std::vector<int> work_sizes = {1, 2, 4, 8, 16, 32, 64, 128, 256};

  auto &storage = Adapters::PrimBenchStorage<Adapters::ActiveBackend>::instance();
  for (const auto &factory : storage.factories()) {
    if (!try_workload(factory)) {
      std::cerr << "Skipping workload (failed probe at work_size=1)\n";
      continue;
    }
    for (int w : work_sizes) {
      exec.queue<WorkloadBridge>(factory(), w);
    }
  }
  exec.run();
  return 0;
}