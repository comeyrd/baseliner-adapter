#ifndef ADAPTER_PRIMBENCHREGISTER_HPP
#define ADAPTER_PRIMBENCHREGISTER_HPP
// adapters/PrimBenchRegister.hpp
#include <adapters/registrars/PrimbenchRegistrar.hpp>

#if defined(BASELINER_BACKEND_HIP)
#include <baseliner/core/hardware/hip/HipBackend.hpp>
namespace Adapters {
  using ActiveBackend = Baseliner::Hardware::HipBackend;
}
#elif defined(BASELINER_BACKEND_CUDA)
#include <baseliner/core/hardware/cuda/CudaBackend.hpp>
namespace Adapters {
  using ActiveBackend = Baseliner::Hardware::CudaBackend;
}
#else
#error "Define BASELINER_BACKEND_HIP or BASELINER_BACKEND_CUDA"
#endif

namespace Adapters {
  using ActiveWorkload = Baseliner::IWorkload<ActiveBackend>;
}

#define PRIMBENCH_REGISTER_WORKLOAD_IMPL(WorkloadType)                                                                 \
  namespace {                                                                                                          \
    static const Adapters::RegisterWorkload<WorkloadType, Adapters::ActiveBackend> _primbench_reg_##WorkloadType{};    \
  }

#define PRIMBENCH_REGISTER_WORKLOAD(W) PRIMBENCH_REGISTER_WORKLOAD_IMPL(W)
#define PRIMBENCH_REGISTER_WORKLOAD_NAME(W, n) PRIMBENCH_REGISTER_WORKLOAD_IMPL(W)
#endif // ADAPTER_PRIMBENCHREGISTER_HPP