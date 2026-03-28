module;

#include <chrono>

export module taco.time;

auto tacoStartTime = std::chrono::system_clock::now();

export int32_t GetTime() {
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now() - tacoStartTime);
  return static_cast<int32_t>(milliseconds.count());
}