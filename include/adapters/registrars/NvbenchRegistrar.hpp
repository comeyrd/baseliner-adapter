// NvbenchRegistrar.hpp
#pragma once
#include <memory>
#include <nvbench/nvbench.cuh>

namespace Adapters {

  // Wraps a raw cudaStream_t into a shared_ptr baseliner expects,
  // without taking ownership (nvbench owns the stream lifetime)
  inline auto make_baseliner_stream(cudaStream_t raw) -> std::shared_ptr<cudaStream_t> {
    return std::shared_ptr<cudaStream_t>(new cudaStream_t(raw),
                                         [](cudaStream_t *p) { delete p; } // owns the ptr, not the stream
    );
  }

  template <class WorkloadT>
  class NvbenchRegistrar {
  public:
    static void run(nvbench::state &state) {
      WorkloadT workload;

      cudaStream_t raw = state.get_cuda_stream();
      auto stream = make_baseliner_stream(raw);

      workload.setup(stream);

      state.exec(nvbench::exec_tag::sync, [&](nvbench::launch &launch) {
        cudaStream_t iter_raw = launch.get_stream();
        auto iter_stream = make_baseliner_stream(iter_raw);

        workload.reset_workload(iter_stream);
        workload.run_workload(iter_stream);
      });

      workload.teardown(stream);
    }
  };

} // namespace Adapters