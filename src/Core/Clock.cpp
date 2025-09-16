#include <R-Engine/Core/Clock.hpp>

/**
* public
*/

void r::core::Clock::tick() noexcept
{
    const TimePoint current_time = SystemClock::now();

    _frame.delta_time = TimeDuration(current_time - _last.frame_time).count();
    _frame.global_time += _frame.delta_time;
    _last.frame_time = current_time;
    _frame.substep_count = 0;
    _last.remainder_time += _frame.delta_time;

    while (_last.remainder_time >= _frame.substep_time) {
        ++_frame.substep_count;
        _last.remainder_time -= _frame.substep_time;
    }
}

const r::core::FrameTime &r::core::Clock::frame() const noexcept
{
    return _frame;
}

/**
* private
*/
