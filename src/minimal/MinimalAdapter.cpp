#include <adapters/registrars/MinimalRegistrar.hpp>
#include <algorithm>
#include <baseliner/core/Durations.hpp>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <vector>

void dump_stats(const std::string &name, std::vector<Baseliner::float_milliseconds> &results) {
  if (results.empty()) {
    return;
  }

  std::vector<float> temp;
  temp.reserve(results.size());
  for (auto single_result : results) {
    temp.push_back(single_result.count());
  }

  std::sort(temp.begin(), temp.end());
  size_t vec_len = temp.size();
  float med = (vec_len % 2 == 0) ? (temp[vec_len / 2 - 1] + temp[vec_len / 2]) / 2.0f : temp[vec_len / 2];

  float mean = std::accumulate(temp.begin(), temp.end(), 0.0f) / vec_len;

  float variance = 0;
  for (float val : temp) {
    variance += (val - mean) * (val - mean);
  }
  float stddev = std::sqrt(variance / vec_len);
  float noise = (mean > 0.0f) ? (stddev / mean) * 100.0f : 0.0f;

  std::cout << std::left << std::setw(30) << name << " | Mean: " << std::fixed << std::setprecision(3) << std::setw(8)
            << mean << " ms"
            << " | Med: " << std::setw(8) << med << " ms"
            << " | Noise: " << std::setw(6) << noise << "\n";
}

int main(int argc, char *argv[]) {
  const auto &workloads = Adapters::GlobalWorkloadStorage::instance().get_workloads();
  if (argc > 1) {
    if (std::string(argv[1]) == "-h") {
      std::cout << "Usage : " << argv[0] << " [nb_repetitions = 50] [work_size = 10] timed\n";
      return 0;
    }
  }
  bool timed = argc > 3;
  uint repetitions = 50;
  int work_size = 10;
  if (argc > 1) {
    try {
      int parsed = std::stoi(argv[1]);
      if (parsed > 0) {
        repetitions = std::min(static_cast<unsigned int>(parsed), 2000u);
      }
    } catch (...) {
    }
  }
  if (argc > 2) {
    try {
      int parsed = std::stoi(argv[2]);
      if (parsed > 0) {
        work_size = std::min(static_cast<int>(parsed), 2000);
      }
    } catch (...) {
    }
  }
  std::cout << "Setup : " << repetitions << " repetitions | " << work_size << " work size | ";
  if (!timed) {
    std::cout << "not ";
  }
  std::cout << "timed\n";
  for (const auto &bridge : workloads) {
    try {
      if (bridge) {
        if (timed) {
          auto results = bridge->timed_run(work_size, repetitions);
          dump_stats(bridge->name(), results);
        } else {
          bridge->run(work_size, repetitions);
          std::cout << bridge->name() << "\n";
        }
      }
    } catch (std::exception &e) {
      std::cout << e.what() << "\n";
    } catch (...) {
      std::cout << "Caught unknown error\n";
    }
  }

  return 0;
}