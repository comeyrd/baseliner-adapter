#include <adapters/registrars/MinimalRegistrar.hpp>
#include <baseliner/core/Durations.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

  constexpr int kMaxRepetitions = 2000;
  constexpr int kMaxWorkSize = 2000;
  constexpr int kMaxDevice = 10;
  constexpr unsigned int kDefaultRepetitions = 50U;
  constexpr int kDefaultWorkSize = 10;
  constexpr int kDefaultDevice = 0;
  constexpr int kNameColumnWidth = 30;
  constexpr int kValueColumnWidth = 8;
  constexpr int kStatPrecision = 3;
  constexpr int kNoisePrecision = 2;
  constexpr float kPercentMultiplier = 100.0F;
  constexpr float kHalf = 0.5F;

  struct Stats {
    float mean;
    float median;
    float stddev;
    float min;
    float max;
    float noise_pct;
    std::size_t count;
  };

  auto compute_stats(const std::vector<float> &sorted_samples) -> Stats {
    const std::size_t sample_count = sorted_samples.size();
    const float mean =
        std::accumulate(sorted_samples.begin(), sorted_samples.end(), 0.0F) / static_cast<float>(sample_count);

    float variance_sum = 0.0F;
    for (const float sample : sorted_samples) {
      const float delta = sample - mean;
      variance_sum += delta * delta;
    }

    const float stddev = (sample_count > 1) ? std::sqrt(variance_sum / static_cast<float>(sample_count - 1)) : 0.0F;

    const float median = (sample_count % 2 == 0)
                             ? (sorted_samples[(sample_count / 2) - 1] + sorted_samples[sample_count / 2]) * kHalf
                             : sorted_samples[sample_count / 2];

    const float noise_pct = (mean > 0.0F) ? (stddev / mean) * kPercentMultiplier : 0.0F;

    return Stats{mean, median, stddev, sorted_samples.front(), sorted_samples.back(), noise_pct, sample_count};
  }

  void print_stats(const std::string &name, const Stats &stats) {
    std::cout << std::left << std::setw(kNameColumnWidth) << name << std::fixed << std::setprecision(kStatPrecision)
              << " | mean " << std::setw(kValueColumnWidth) << stats.mean << " | med " << std::setw(kValueColumnWidth)
              << stats.median << " | min " << std::setw(kValueColumnWidth) << stats.min << " | max "
              << std::setw(kValueColumnWidth) << stats.max << " | noise " << std::setprecision(kNoisePrecision)
              << stats.noise_pct << "%\n";
  }

  void write_json(std::ostream &out, const std::string &name, const Stats &stats, const std::vector<float> &samples) {
    out << "  {\"name\": \"" << name << "\"" << ", \"count\": " << stats.count << ", \"mean_ms\": " << stats.mean
        << ", \"median_ms\": " << stats.median << ", \"min_ms\": " << stats.min << ", \"max_ms\": " << stats.max
        << ", \"stddev_ms\": " << stats.stddev << ", \"noise_pct\": " << stats.noise_pct << ", \"samples_ms\": [";
    for (std::size_t index = 0; index < samples.size(); ++index) {
      if (index != 0U) {
        out << ", ";
      }
      out << samples[index];
    }
    out << "]}";
  }

  auto parse_clamped_int(const char *argument, int max_value, int default_value) -> int {
    try {
      const int parsed = std::stoi(argument);
      return std::min(parsed, max_value);
    } catch (const std::exception &) {
      return default_value;
    }
  }
} // namespace

auto main(int argc, char *argv[]) -> int {
  const std::vector<std::string> arguments(argv, argv + argc);

  if (arguments.size() > 1 && arguments[1] == "-h") {
    std::cout << "Usage: " << arguments[0] << " [repetitions=50] [work_size=10] [device=0] [timed] [json_path]\n";
    return EXIT_SUCCESS;
  }

  const unsigned int repetitions =
      (arguments.size() > 1)
          ? static_cast<unsigned int>(parse_clamped_int(arguments[1].c_str(), kMaxRepetitions, kDefaultRepetitions))
          : kDefaultRepetitions;
  const int work_size = (arguments.size() > 2) ? parse_clamped_int(arguments[2].c_str(), kMaxWorkSize, kDefaultWorkSize)
                                               : kDefaultWorkSize;
  const int device =
      (arguments.size() > 3) ? parse_clamped_int(arguments[3].c_str(), kMaxDevice, kDefaultDevice) : kDefaultDevice;
  const bool timed = arguments.size() > 4;
  const std::string json_path = (arguments.size() > 5) ? arguments[5] : std::string{};

  std::cout << "Setup: " << repetitions << " reps | " << work_size << " work_size | " << "device " << device << " | "
            << (timed ? "timed" : "not timed") << "\n";

  std::ofstream json_out;
  if (!json_path.empty()) {
    json_out.open(json_path);
    json_out << "[\n";
  }
  bool is_first_entry = true;

  for (const auto &bridge : Adapters::GlobalWorkloadStorage::instance().get_workloads()) {
    if (!bridge) {
      continue;
    }
    try {
      if (!timed) {
        bridge->run(work_size, repetitions, device);
        std::cout << bridge->name() << "\n";
        continue;
      }

      const auto results = bridge->timed_run(work_size, repetitions, device);
      if (results.empty()) {
        continue;
      }

      std::vector<float> samples;
      samples.reserve(results.size());
      for (const auto &result : results) {
        samples.push_back(result.count());
      }

      std::vector<float> sorted_samples = samples;
      std::sort(sorted_samples.begin(), sorted_samples.end());

      const Stats stats = compute_stats(sorted_samples);
      print_stats(bridge->name(), stats);

      if (json_out.is_open()) {
        if (!is_first_entry) {
          json_out << ",\n";
        }
        write_json(json_out, bridge->name(), stats, samples);
        is_first_entry = false;
      }
    } catch (const std::exception &error) {
      std::cout << error.what() << "\n";
    }
  }

  if (json_out.is_open()) {
    std::cout << "saving to : " << json_path << "\n";
    json_out << "\n]\n";
  }
  return EXIT_SUCCESS;
}