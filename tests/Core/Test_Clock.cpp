#include "../Test.hpp"

#include <R-Engine/Core/Clock.hpp>

#include <thread>

using namespace r::core;

Test(Clock, initial_frame_values)
{
    Clock clock;
    const FrameTime &frame = clock.frame();

    cr_expect_eq(frame.delta_time, 0.0f, "Expected delta_time to be 0.0f");
    cr_expect_eq(frame.global_time, 0.0f, "Expected global_time to be 0.0f");
    cr_expect_eq(frame.substep_count, 0, "Expected substep_count to be 0");
    cr_expect_eq(frame.substep_time, 0.016666f, "Expected substep_time to be 0.016666f");
}

Test(Clock, tick_increases_delta_time)
{
    Clock clock;

    const FrameTime &frame_before = clock.frame();
    cr_expect_eq(frame_before.delta_time, 0.0f);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    clock.tick();

    const FrameTime &frame_after = clock.frame();

    cr_expect(frame_after.delta_time > 0.0f, "Expected delta_time to be greater than 0 after tick");
    cr_expect(frame_after.global_time > 0.0f, "Expected global_time to be greater than 0 after tick");
}

Test(Clock, multiple_ticks_accumulate_time)
{
    Clock clock;

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    clock.tick();
    const f32 first_delta = clock.frame().delta_time;

    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    clock.tick();
    const f32 second_delta = clock.frame().delta_time;

    cr_expect(second_delta > first_delta, "Expected delta_time to increase with multiple ticks");
}
