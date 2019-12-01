#pragma once
#include <chrono>
#include <utility>
#include <string>

class Timer {
  std::chrono::system_clock::time_point start;
public:
  void restart() {
    start = std::chrono::high_resolution_clock::now();
  }
  double end() {
    auto e = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = e - start;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    return ms.count();
  }
  std::string toString() {
    return "ran for: " + std::to_string(end());
  }
};

