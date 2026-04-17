module;

#include <chrono>

export module taco.time;

auto taco_start_time = std::chrono::system_clock::now();

export int32_t GetTime() {
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now() - taco_start_time);
  return static_cast<int32_t>(milliseconds.count());
}
