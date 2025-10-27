#include <R-Engine/Application.hpp>
#include <R-Engine/Components/Transform3d.hpp>
#include <R-Engine/Core/Clock.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/Maths/Vec.hpp>

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

constexpr i32 NUM_ENTITIES = 100000;
constexpr i32 BENCHMARK_DURATION_SECONDS = 10;

struct Velocity {
        r::Vec3f value;
};
struct Acceleration {
        r::Vec3f value;
};
struct HeavyData {
        f32 data[16];
};
struct BenchmarkState {
        std::chrono::steady_clock::time_point start_time;
        u64 update_frame_count = 0;
        u64 fixed_update_step_count = 0;
        bool started = false;
};

static void setup_system(r::ecs::Commands &commands)
{
    r::Logger::info("Benchmark Setup: Spawning " + std::to_string(NUM_ENTITIES) + " entities...");
    srand(time(nullptr));

    for (i32 i = 0; i < NUM_ENTITIES; ++i) {
        auto entity =
            commands.spawn(r::Transform3d{.position = {(f32) (rand() % 200 - 100), (f32) (rand() % 200 - 100), (f32) (rand() % 200 - 100)}},
                Velocity{.value = {(f32) (rand() % 20 - 10), (f32) (rand() % 20 - 10), 0.0f}});

        if (i % 2 == 0) {
            entity.insert(Acceleration{.value = {0.0f, -9.81f, 0.0f}});
        }
        if (i % 3 == 0) {
            entity.insert(HeavyData{});
        }
    }
}

static void move_system(r::ecs::Query<r::ecs::Ref<Velocity>, r::ecs::Mut<r::Transform3d>> query, r::ecs::Res<r::core::FrameTime> time)
{
    for (const auto &[vel, trans] : query) {
        trans.ptr->position += vel.ptr->value * time.ptr->delta_time;
    }
}
static void rotate_system(r::ecs::Query<r::ecs::Mut<r::Transform3d>> query, r::ecs::Res<r::core::FrameTime> time)
{
    for (const auto &[trans] : query) {
        trans.ptr->rotation.z += 0.1f * time.ptr->delta_time;
    }
}

static void apply_acceleration_system(r::ecs::Query<r::ecs::Ref<Acceleration>, r::ecs::Mut<Velocity>> query,
    r::ecs::Res<r::core::FrameTime> time)
{
    for (const auto &[accel, vel] : query) {
        vel.ptr->value += accel.ptr->value * time.ptr->delta_time;
    }
}

static void heavy_computation_A(r::ecs::Query<r::ecs::Ref<r::Transform3d>> query)
{
    for (const auto &[trans] : query) {

        volatile f32 result = 0.f;
        for (i32 i = 0; i < 20; ++i)
            result += std::sin(trans.ptr->position.x) * std::cos(result);
    }
}
static void heavy_computation_B(r::ecs::Query<r::ecs::Ref<Velocity>> query)
{
    for (const auto &[vel] : query) {
        volatile f32 result = 0.f;
        for (i32 i = 0; i < 20; ++i)
            result += std::sqrt(std::abs(vel.ptr->value.length() + result));
    }
}
static void heavy_computation_C(r::ecs::Query<r::ecs::Ref<HeavyData>> query)
{
    for (const auto &[data] : query) {
        volatile f32 result = 0.f;
        for (i32 i = 0; i < 20; ++i)
            result += std::log(std::abs(data.ptr->data[i % 16] + 1.0f + result));
    }
}

static void fixed_move_system(r::ecs::Query<r::ecs::Ref<Velocity>, r::ecs::Mut<r::Transform3d>> query, r::ecs::Res<r::core::FrameTime> time)
{
    for (const auto &[vel, trans] : query) {
        trans.ptr->position += vel.ptr->value * time.ptr->substep_time;
    }
}
static void count_fixed_steps(r::ecs::ResMut<BenchmarkState> state)
{
    state.ptr->fixed_update_step_count++;
}

static void control_and_report_system(r::ecs::ResMut<BenchmarkState> state)
{
    if (!state.ptr->started) {
        state.ptr->start_time = std::chrono::steady_clock::now();
        state.ptr->started = true;
        return;
    }

    state.ptr->update_frame_count++;

    const auto now = std::chrono::steady_clock::now();
    const auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<const f64>>(now - state.ptr->start_time).count();

    if (elapsed_seconds >= BENCHMARK_DURATION_SECONDS) {
        r::Logger::info("--- Benchmark Finished ---");

        const f64 avg_update_fps = state.ptr->update_frame_count / elapsed_seconds;
        const f64 avg_update_time_ms = (elapsed_seconds * 1000.0) / state.ptr->update_frame_count;
        const f64 avg_fixed_updates_per_second = state.ptr->fixed_update_step_count / elapsed_seconds;

        std::cout << "\n--- Performance Report ---\n";
        std::cout << "Duration:              " << elapsed_seconds << " s\n";
        std::cout << "Entities:              " << NUM_ENTITIES << "\n";
        std::cout << "Total Update frames:   " << state.ptr->update_frame_count << "\n";
        std::cout << "Total FixedUpdate steps: " << state.ptr->fixed_update_step_count << "\n";
        std::cout << "---------------------------\n";
        std::cout << "Avg Update FPS:        " << avg_update_fps << "\n";
        std::cout << "Avg Update Frame Time: " << avg_update_time_ms << " ms\n";
        std::cout << "Avg FixedUpdate/sec:   " << avg_fixed_updates_per_second << "\n";
        std::cout << "---------------------------\n\n";

        r::Application::quit.store(true);
    }
}

i32 main()
{
    r::Logger::info("Starting headless benchmark for " + std::to_string(BENCHMARK_DURATION_SECONDS) + " seconds...");

    r::Application{}
        .insert_resource(BenchmarkState{})
        .add_systems<setup_system>(r::Schedule::STARTUP)
        .add_systems<move_system, rotate_system, apply_acceleration_system, heavy_computation_A, heavy_computation_B, heavy_computation_C,
            control_and_report_system>(r::Schedule::UPDATE)
        .add_systems<fixed_move_system, count_fixed_steps>(r::Schedule::FIXED_UPDATE)
        .run();

    r::Logger::info("Benchmark finished. Exiting.");

    return 0;
}
