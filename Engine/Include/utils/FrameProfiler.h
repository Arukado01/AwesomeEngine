#pragma once

#include <chrono>
#include <cstdint>
#include <fstream>

/* Logger opcional de intervalos de frame.
   Se activa definiendo AE_FRAME_LOG con la ruta del CSV.
   Sin esa variable no abre archivo y Frame() no hace trabajo relevante. */
class FrameProfiler {
public:
  FrameProfiler();
  ~FrameProfiler();

  void Frame();

private:
  using Clock = std::chrono::steady_clock;

  std::ofstream out_;
  Clock::time_point start_{};
  Clock::time_point previous_{};
  std::uint64_t frame_ = 0;
};
