#include "hip/hip_runtime.h"

/* Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Copyright 2026, Come Eyraud.
#include "../ExampleWorkload.hpp"
#include <adapters/Register.hpp>
#include <baseliner/core/HipBackend.hpp>
#include <cstddef>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>
// ----------------------------------------------------------------------
// Original Kernel Implementation (Unchanged Logic)
// ----------------------------------------------------------------------

/**
 * Matrix multiplication (HIP Kernel) on the Backend: C = A * B
 * wA is A's width and wB is B's width
 */
template <int BLOCK_SIZE>
__global__ void MatrixMulHIP(float *C, float *A, float *B, int wA, int wB) {
  // Block index
  int bx = blockIdx.x;
  int by = blockIdx.y;

  // Thread index
  int tx = threadIdx.x;
  int ty = threadIdx.y;

  // Index of the first sub-matrix of A processed by the block
  int aBegin = wA * BLOCK_SIZE * by;

  // Index of the last sub-matrix of A processed by the block
  int aEnd = aBegin + wA - 1;

  // Step size used to iterate through the sub-matrices of A
  int aStep = BLOCK_SIZE;

  // Index of the first sub-matrix of B processed by the block
  int bBegin = BLOCK_SIZE * bx;

  // Step size used to iterate through the sub-matrices of B
  int bStep = BLOCK_SIZE * wB;

  // Csub is used to store the element of the block sub-matrix
  // that is computed by the thread
  float Csub = 0;

  // Loop over all the sub-matrices of A and B
  // required to compute the block sub-matrix
  for (int a = aBegin, b = bBegin; a <= aEnd; a += aStep, b += bStep) {
    // Declaration of the shared memory array As used to
    // store the sub-matrix of A
    __shared__ float As[BLOCK_SIZE][BLOCK_SIZE];

    // Declaration of the shared memory array Bs used to
    // store the sub-matrix of B
    __shared__ float Bs[BLOCK_SIZE][BLOCK_SIZE];

    // Load the matrices from Backend memory
    // to shared memory; each thread loads
    // one element of each matrix
    As[ty][tx] = A[a + wA * ty + tx];
    Bs[ty][tx] = B[b + wB * ty + tx];

    // Synchronize to make sure the matrices are loaded
    __syncthreads();

    // Multiply the two matrices together;
    // each thread computes one element
    // of the block sub-matrix
#pragma unroll

    for (int k = 0; k < BLOCK_SIZE; ++k) {
      Csub += As[ty][k] * Bs[k][tx];
    }

    // Synchronize to make sure that the preceding
    // computation is done before loading two new
    // sub-matrices of A and B in the next iteration
    __syncthreads();
  }

  // Write the block sub-matrix to Backend memory;
  // each thread writes one element
  int c = wB * BLOCK_SIZE * by + BLOCK_SIZE * bx;
  C[c + wB * ty + tx] = Csub;
}
// NOLINTEND
// ----------------------------------------------------------------------
// Interface Implementation
// ----------------------------------------------------------------------

template <>
void MatrixMul<Baseliner::Hardware::HipBackend>::setup(std::shared_ptr<hipStream_t> stream) {
  alloc_host();
  generate_random(get_seed(), m_h_A, m_h_B);
  size_t mem_size_A = m_size_A * sizeof(float);
  size_t mem_size_B = m_size_B * sizeof(float);
  size_t mem_size_C = m_hA * m_wB * sizeof(float);

  CHECK_HIP(hipMalloc(&m_d_A, mem_size_A));
  CHECK_HIP(hipMalloc(&m_d_B, mem_size_B));
  CHECK_HIP(hipMalloc(&m_d_C, mem_size_C));

  CHECK_HIP(hipMemcpyAsync(m_d_A, m_h_A.data(), mem_size_A, hipMemcpyHostToDevice, *stream));
  CHECK_HIP(hipMemcpyAsync(m_d_B, m_h_B.data(), mem_size_B, hipMemcpyHostToDevice, *stream));

  // Setup execution parameters
  int block_size = m_block_size;
  m_threads_1 = block_size;
  m_threads_2 = block_size;

  m_grid_1 = (m_wB + m_threads_1 - 1) / m_threads_1;
  m_grid_2 = (m_hA + m_threads_2 - 1) / m_threads_2;
};
template <>
void MatrixMul<Baseliner::Hardware::HipBackend>::teardown(std::shared_ptr<hipStream_t> stream) {
  size_t mem_size_C = m_hA * m_wB * sizeof(float);
  CHECK_HIP(hipMemcpyAsync(m_h_C.data(), m_d_C, mem_size_C, hipMemcpyDeviceToHost, *stream));

  CHECK_HIP(hipFree(m_d_A));
  CHECK_HIP(hipFree(m_d_B));
  CHECK_HIP(hipFree(m_d_C));
};
template <>
void MatrixMul<Baseliner::Hardware::HipBackend>::reset_workload(std::shared_ptr<hipStream_t> stream) {
  size_t mem_size_C = m_hA * m_wB * sizeof(float);
  CHECK_HIP(hipMemsetAsync(m_d_C, 0, mem_size_C, *stream));
};
template <>
auto MatrixMul<Baseliner::Hardware::HipBackend>::run_workload(std::shared_ptr<hipStream_t> stream) -> std::monostate {
  auto grid = dim3(m_grid_1, m_grid_2);
  auto thread = dim3(m_threads_1, m_threads_2);
  if (m_block_size == 16) { // NOLINT
    MatrixMulHIP<16>        // NOLINT
        <<<grid, thread, 0, *stream>>>(m_d_C, m_d_A, m_d_B, m_wA, m_wB);
  } else {
    MatrixMulHIP<32> // NOLINT
        <<<grid, thread, 0, *stream>>>(m_d_C, m_d_A, m_d_B, m_wA, m_wB);
  }
  return {};
}
using MatrixMulHip = MatrixMul<Baseliner::Hardware::HipBackend>;

BASELINER_REGISTER_WORKLOAD(MatrixMulHip)
