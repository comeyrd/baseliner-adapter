#include <adapters/registrars/MinimalRegistrar.hpp>

int main(int argc, char *argv[]) {
  const auto &workloads = Adapters::GlobalWorkloadStorage::instance().get_workloads();

  for (const auto &bridge : workloads) {
    if (bridge) {
      bridge->run();
    }
  }

  return 0;
}