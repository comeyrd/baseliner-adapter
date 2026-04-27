#pragma once

#ifdef __GNUC__
#define ADAPTER_ATTRIBUTE_USED __attribute__((used))
#else
#define ADAPTER_ATTRIBUTE_USED
#endif

// ── Baseliner backend ──────────────────────────────────────────────
#if defined(ADAPTER_BACKEND_BASELINER)
#include <adapters/registrars/BaselinerRegistrar.hpp>

#define BASELINER_REGISTER_WORKLOAD(W)                                                                                 \
  ADAPTER_ATTRIBUTE_USED                                                                                               \
  static Adapters::BaselinerRegistrar<W> _adapter_registrar_##W##__LINE__{#W};

// ── NVBench backend ────────────────────────────────────────────────
#elif defined(ADAPTER_BACKEND_NVBENCH)
#include <adapters/registrars/NvbenchRegistrar.hpp>

// nvbench needs NVBENCH_BENCH at file scope, so the macro does more here
#define BASELINER_REGISTER_WORKLOAD(W)                                                                                 \
  static void _##W##_nvbench_fn(nvbench::state &state) {                                                               \
    Adapters::NvbenchRegistrar<W>::run(state);                                                                         \
  }                                                                                                                    \
  NVBENCH_BENCH(_##W##_nvbench_fn);

// ── Primbench backend ──────────────────────────────────────────────
#elif defined(ADAPTER_BACKEND_PRIMBENCH)
#include <adapters/registrars/PrimbenchRegistrar.hpp>

#define BASELINER_REGISTER_WORKLOAD(W)                                                                                 \
  ADAPTER_ATTRIBUTE_USED                                                                                               \
  static Adapters::PrimbenchRegistrar<W> _adapter_primbench_registrar_##W##_##__LINE__{};

#ifndef ADAPTER_PRIMBENCH_MAIN_DEFINED
#define ADAPTER_PRIMBENCH_MAIN_DEFINED
int main(int argc, char *argv[]) {
  return Adapters::PrimbenchMain(argc, argv);
}
#endif
#else
#error                                                                                                                 \
    "No adapter backend defined. Set ADAPTER_BACKEND_BASELINER, ADAPTER_BACKEND_NVBENCH, or ADAPTER_BACKEND_PRIMBENCH"
#endif