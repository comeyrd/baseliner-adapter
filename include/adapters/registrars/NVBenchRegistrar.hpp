// NvbenchRegistrar.hpp
#pragma once
#include <memory>
#include <nvbench/nvbench.cuh>

namespace Adapters {

  template <class WorkloadT>
  class NvbenchRegistrar {
  public:
    static void run(nvbench::state &state) {
      WorkloadT workload;

      workload.setup_host();

      workload.setup_device(state.get_stream());

      state.exec(nvbench::exec_tag::sync, [&](nvbench::launch &launch) {
        workload.reset_device(launch.get_stream());
        workload.run(launch.get_stream());
      });

      workload.fetch_results(state.get_stream());
      workload.free();
    }
  };

} // namespace Adapters