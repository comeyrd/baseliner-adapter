# Baseliner Adapters

A collection of adapters that enable [Baseliner](https://github.com/comeyrd/gpu-kernel-baseliner) workloads to run seamlessly across multiple GPU benchmarking frameworks (NVBench, PrimBench, and minimal profiler wrappers) without code modification.

**Version:** 1.0

## Dependencies

- **Baseliner Core:** v1.0
- **NVBench:** commit `a3364ca5c787da0bd6ef83683e3d42c8d730d579`
- **PrimBench:** commit `dd6042eab069d7e6c4e777011397237a4fa1a81b` (from ROCm/rocm-libraries)

## Overview

The adapter mechanism solves a critical problem in GPU benchmarking: **workload lock-in**. Traditionally, implementing a benchmark for one framework (e.g., NVBench) means it cannot run on another (e.g., PrimBench) without significant rewriting. Baseliner adapters eliminate this friction by providing a unified interface that wraps vendor-specific benchmarking tools.

### Key Features

- **Write Once, Run Everywhere**: Define a workload using Baseliner's interface and run it on NVBench, PrimBench, or minimal profiler wrappers
- **Hardware Agnostic**: Full support for both CUDA and HIP backends
- **Zero Overhead**: Adapters introduce no measurable performance penalty (< 1μs median deviation)
- **Header-Only Integration**: Minimal dependencies and simple build process

## Architecture

The adapter architecture consists of three layers:

1. **Baseliner Core Interface**: The standardized GPU workload lifecycle (allocation → transfer → execution → retrieval)
2. **Adapter Layer**: Thin wrappers that translate Baseliner workloads into framework-specific formats
3. **Target Frameworks**: NVBench (NVIDIA), PrimBench (AMD/NVIDIA), or minimal profiler wrappers

```
┌─────────────────────────────────────┐
│   Your Workload (MatMul, GEMM...)  │
│  (implements IWorkload<Backend>)    │
└──────────────┬──────────────────────┘
               │
    ┌──────────┴─────────────┬──────────────┐
    │                        │              │
┌───▼────────┐      ┌────────▼───┐   ┌─────▼──────┐
│  NVBench   │      │ PrimBench  │   │  Minimal   │
│  Adapter   │      │  Adapter   │   │  Adapter   │
└────────────┘      └────────────┘   └────────────┘
```

## Dependencies

### Required

- **CMake** ≥ 3.15
- **C++17** compatible compiler
- **Baseliner** (automatically fetched from [gpu-kernel-baseliner](https://github.com/comeyrd/gpu-kernel-baseliner))

### Optional (Backend-Specific)

#### For CUDA Support
- CUDA Toolkit ≥ 12.0
- **NVBench** (automatically fetched)
- NVML (optional, for GPU monitoring)

#### For HIP Support
- ROCm ≥ 5.0
- **PrimBench** header (automatically fetched)
- AMD SMI (optional, for GPU monitoring at `/opt/rocm`)

#### For Both
- **PrimBench** header (automatically fetched for cross-vendor support)

## Building

### Quick Start

```bash
# Clone the repository
git clone https://github.com/comeyrd/baseliner-adapter.git
cd baseliner-adapter

# Configure with CUDA backend
cmake -B build -DCMAKE_BUILD_TYPE=Release \
               -DADAPTERS_BUILD_EXAMPLES=ON

# Build
cmake --build build -j$(nproc)
```

### Backend Selection

The build system automatically detects available GPU compilers (CUDA/HIP):

```bash
# Force HIP backend for PrimBench adapter
cmake -B build -DADAPTERS_PRIMBENCH_BACKEND=HIP

# Force CUDA backend for PrimBench adapter
cmake -B build -DADAPTERS_PRIMBENCH_BACKEND=CUDA
```

### CMake Presets

Use the included presets for common configurations:

```bash
# Debug build with CUDA
cmake --preset debug-cuda
cmake --build --preset debug-cuda

# Release build with HIP
cmake --preset release-hip
cmake --build --preset release-hip
```

## Usage

### Defining a Workload

Create a workload by implementing the `Baseliner::IWorkload<Backend>` interface:

```cpp
#include <baseliner/core/Workload.hpp>

template <typename BackendT>
class MyWorkload : public Baseliner::IWorkload<BackendT> {
public:
  using backend = typename MyWorkload::backend;

  auto algo() -> std::string override {
    return "MyWorkload";
  }

  // Define the six lifecycle stages:
  void setup_host_random_generated() override { /* ... */ }
  void setup_device(typename backend::stream_t stream) override { /* ... */ }
  void reset_device(typename backend::stream_t stream) override { /* ... */ }
  auto run(typename backend::stream_t stream)
    -> typename backend::launch_result_t override { /* ... */ }
  void fetch_results(typename backend::stream_t stream) override { /* ... */ }
  void free() override { /* ... */ }

  auto validate() -> bool override { /* ... */ }
};
```
### Registering Components : 
```cpp
#include <baseliner/Register.hpp>
namespace {
  BASELINER_REGISTER_WORKLOAD(MyWorkload);
  BASELINER_REGISTER_STOPPING_CRITERION(MyCriterion);
  BASELINER_REGISTER_STAT(MyStat);
  BASELINER_REGISTER_BACKEND("mybackend", MyBackend);
}
```

### CMake Integration

```cmake
# Add baseliner-adapter to your project using FetchContent
include(FetchContent)

FetchContent_Declare(
  baseliner_adapters
  GIT_REPOSITORY https://github.com/comeyrd/baseliner-adapter.git
  GIT_TAG        v1.0
)
FetchContent_MakeAvailable(baseliner_adapters)

# Link your workload against the desired adapter
add_executable(my_benchmark main.cpp)
target_link_libraries(my_benchmark PRIVATE
  baseliner_adapters::nvbench  # or ::primbench, ::baseliner, ::minimal
)
```

## Examples

The repository includes a complete matrix multiplication example demonstrating all adapters:

```bash
# Build examples
cmake -B build -DADAPTERS_BUILD_EXAMPLES=ON
cmake --build build

# Run with different adapters
./build/examples/matmul_nvbench    # NVBench
./build/examples/matmul_primbench  # PrimBench
./build/examples/matmul_baseliner  # Baseliner
./build/examples/matmul_minimal    # Minimal
```

Example source structure:
- `examples/MatMulWorkload/MatMulWorkload.hpp` - Shared workload definition
- `examples/MatMulWorkload/cuda/MatMulWorkload.cu` - CUDA implementation
- `examples/MatMulWorkload/hip/MatMulWorkload.hip` - HIP implementation

## Real-World Usage: blas-baseliner

The [blas-baseliner](https://github.com/comeyrd/blas-baseliner) project demonstrates production use of these adapters. It benchmarks cuBLAS and rocBLAS operations (GEMM, SAXPY, etc.) across NVIDIA and AMD hardware using a single codebase.
This demonstrates the adapters' ability to maximize code reuse while maintaining vendor-specific optimizations.