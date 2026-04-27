#pragma once
#include <functional>
#include <iostream>
#include <memory>
#include <primbench.hpp>
#include <vector>

namespace Adapters {

  inline std::vector<std::function<void(primbench::executor &)>> &PrimbenchWorkloadRegistry() {
    static std::vector<std::function<void(primbench::executor &)>> registry;
    return registry;
  }

  template <class WorkloadT>
  class PrimbenchRegistrar {
  public:
    PrimbenchRegistrar() {
      PrimbenchWorkloadRegistry().push_back([](primbench::executor &exec) { exec.queue<WorkloadBridge>(); });
    }

  private:
    struct WorkloadBridge : public primbench::benchmark_interface {
      mutable WorkloadT workload;

      primbench::json meta() const override {
        return primbench::json{}.add("algo", workload.name());
      }

      void run(primbench::state &state) override {
        auto stream = std::shared_ptr<hipStream_t>(&const_cast<hipStream_t &>(state.stream), [](hipStream_t *) {});

        workload.setup(stream);

        auto flops = workload.number_of_floating_point_operations();
        if (flops.has_value() && flops.value() > 0) {
          state.set_items(flops.value());
        } else {
          state.set_items(state.size);
          std::cout << "none" << "\n";
        }
        auto bytes = workload.number_of_bytes();
        if (bytes.has_value()) {
          state.add_reads<uint8_t>(bytes.value());  // A + B
          state.add_writes<uint8_t>(bytes.value()); // C (overestimate, but fine)
        }

        state.run([&]() {
          workload.reset_workload(stream);
          workload.run_workload(stream);
        });

        workload.teardown(stream);
      }
    };
  };

  inline int PrimbenchMain(int argc, char *argv[]) {
    primbench::executor exec(argc, argv);
    for (auto &enqueue : PrimbenchWorkloadRegistry()) {
      enqueue(exec);
    }
    exec.run();
    return 0;
  }

} // namespace Adapters