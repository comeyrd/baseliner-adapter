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

#ifndef MATRIXMUL_KERNEL_HPP
#define MATRIXMUL_KERNEL_HPP

#include <baseliner/specs/Options.hpp>
#include <baseliner/specs/Workload.hpp>

#include <cstddef>
#include <iostream>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <variant>
#include <vector>
inline void generate_random(int seed, std::vector<float> &m_h_A, std::vector<float> &m_h_B) {
  std::mt19937 gen(seed);
  std::uniform_real_distribution<float> dist(0.0f, 1.0f); // NOLINT

  for (auto &val : m_h_A) {
    val = dist(gen);
  }
  for (auto &val : m_h_B) {
    val = dist(gen);
  }
};

template <typename BackendT>
class MatrixMul : public Baseliner::IWorkload<BackendT> {
public:
  auto name() -> std::string override {
    return "MatrixMulKernel";
  };
  void alloc_host() {
    m_wA = m_wA_base;
    m_hA = m_hA_base * this->get_work_size();

    // Inner dimensions must match
    m_hB = m_wA;
    m_wB = m_wB_base;

    m_size_A = m_wA * m_hA;
    m_size_B = m_wB * m_hB;

    m_h_A.resize(m_size_A);
    m_h_B.resize(m_size_B);
    m_size_C = m_hA * m_wB;
    m_h_C.resize(m_size_C);
  };
  void setup(std::shared_ptr<typename BackendT::stream_t> stream) override;

  void reset_workload(std::shared_ptr<typename BackendT::stream_t> stream) override;

  auto run_workload(std::shared_ptr<typename BackendT::stream_t> stream) -> std::monostate override;

  void teardown(std::shared_ptr<typename BackendT::stream_t> stream) override;

  auto number_of_floating_point_operations() -> std::optional<size_t> override {
    size_t flops = 2ULL * m_hA * m_wA * m_wB;
    return flops;
  }
  auto number_of_bytes() -> std::optional<size_t> override {
    size_t bytes = sizeof(float) * (m_size_A + m_size_B + m_size_C);
    return bytes;
  }
  auto validate_workload() -> bool override {
    return true;
  }

protected:
  void register_options() override {
    Baseliner::IWorkload<BackendT>::register_options();
    this->add_option("MatrixMulInput", "wA", "Width of Matrix A", m_wA_base);
    this->add_option("MatrixMulInput", "hA", "Height of Matrix A", m_hA_base);
    this->add_option("MatrixMulInput", "wB", "Width of Matrix B", m_wB_base);
    this->add_option("MatrixMulInput", "hB", "Height of Matrix B", m_hB_base);
    this->add_option("MatrixMulInput", "block_size", "Block size (16 or 32)", m_block_size);
  };

private:
  int m_wA_base = 1024; // NOLINT
  int m_hA_base = 1024; // NOLINT
  int m_wB_base = 1024; // NOLINT
  int m_hB_base = 1024; // NOLINT

  int m_wA, m_hA, m_wB, m_hB;
  int m_size_A, m_size_B;
  int m_block_size = 16;

  std::vector<float> m_h_A;
  std::vector<float> m_h_B;

  float *m_d_A = nullptr;
  float *m_d_B = nullptr;
  float *m_d_C = nullptr;
  std::vector<float> m_h_C;
  int m_size_C;
  int m_threads_1;
  int m_threads_2;
  int m_grid_1;
  int m_grid_2;
};

#endif // MATRIXMUL_KERNEL_HPP