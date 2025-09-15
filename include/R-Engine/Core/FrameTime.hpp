#pragma once

#include <R-Engine/Types.hpp>
#include <chrono>

namespace r {

namespace core {

using SystemClock = std::chrono::system_clock;
using TimePoint = std::chrono::time_point<SystemClock>;
using TimeDuration = std::chrono::duration<f32>;

// clang-format off
struct FrameTime {
    f32 delta_time = .0;
    f32 global_time = .0;
    i32 substep_count = 0;
    f32 substep_time = .016666f;
};

struct LastTime {
    TimePoint frame_time = SystemClock::now();
    f32 remainder_time = .0;
};
// clang-format on

}// namespace core

}// namespace r
