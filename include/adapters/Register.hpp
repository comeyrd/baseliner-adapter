#pragma once

#ifdef __GNUC__
  #define ADAPTER_ATTRIBUTE_USED __attribute__((used))
#else
  #define ADAPTER_ATTRIBUTE_USED
#endif

// ── Baseliner backend ──────────────────────────────────────────────
#if defined(ADAPTER_BACKEND_BASELINER)
  #include <adapters/registrars/BaselinerRegistrar.hpp>

  #define ADAPTER_REGISTER_CUDA_WORKLOAD(W)                              \
    ADAPTER_ATTRIBUTE_USED                                               \
    static Adapters::BaselinerRegistrar<W>                               \
        _adapter_registrar_##W##__LINE__{#W};

  #define ADAPTER_REGISTER_HIP_WORKLOAD(W)                               \
    ADAPTER_ATTRIBUTE_USED                                               \
    static Adapters::BaselinerRegistrar<W>                               \
        _adapter_registrar_##W##__LINE__{#W};

// ── NVBench backend ────────────────────────────────────────────────
#elif defined(ADAPTER_BACKEND_NVBENCH)
  #include <adapters/registrars/NvbenchRegistrar.hpp>

  // nvbench needs NVBENCH_BENCH at file scope, so the macro does more here
  #define ADAPTER_REGISTER_CUDA_WORKLOAD(W)                              \
    static void _##W##_nvbench_fn(nvbench::state & state) {             \
      Adapters::NvbenchRegistrar<W>::run(state);                         \
    }                                                                    \
    NVBENCH_BENCH(_##W##_nvbench_fn);

  #define ADAPTER_REGISTER_HIP_WORKLOAD(W) // nvbench is CUDA only

// ── Primbench backend ──────────────────────────────────────────────
#elif defined(ADAPTER_BACKEND_PRIMBENCH)
  #include <adapters/registrars/PrimbenchRegistrar.hpp>

  // primbench needs main() at file scope
  #define ADAPTER_REGISTER_HIP_WORKLOAD(W)                               \
    int main(int argc, char *argv[]) {                                   \
      return Adapters::PrimbenchRegistrar<W>::run(argc, argv);           \
    }

  #define ADAPTER_REGISTER_CUDA_WORKLOAD(W) // primbench is HIP only

#else
  #error "No adapter backend defined. Set ADAPTER_BACKEND_BASELINER, ADAPTER_BACKEND_NVBENCH, or ADAPTER_BACKEND_PRIMBENCH"
#endif