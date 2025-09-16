#include <R-Engine/Application.hpp>
#include <R-Engine/ECS/Command.hpp>

#include <iostream>

struct Position {
        f32 x, y = 0;
};

struct Velocity {
        f32 vx = 0, vy = 0;
};

using namespace r;
using namespace ecs;

static void spawn_startup(Commands cmds)
{
    const Entity e1 = cmds.spawn();
    cmds.add_component<Position>(e1, Position{0.0f, 0.0f});
    cmds.add_component<Velocity>(e1, Velocity{1.0f, 0.5f});

    const Entity e2 = cmds.spawn();
    cmds.add_component<Position>(e2, Position{10.0f, -5.0f});
    cmds.add_component<Velocity>(e2, Velocity{-0.2f, 0.0f});
}

// static void setup_systems(Res<core::FrameTime> time, Query<Mut<Position>, Ref<Velocity>> q)
// {
//     for (const auto &tup : q) {
//         const auto &pos_mut = std::get<0>(tup);
//         const auto &vel_ref = std::get<1>(tup);
//
//         if (pos_mut.ptr && vel_ref.ptr) {
//             pos_mut.ptr->x += vel_ref.ptr->vx * time.ptr->delta_time;
//             pos_mut.ptr->y += vel_ref.ptr->vy * time.ptr->delta_time;
//         }
//     }
// }

// static void print_systems(Query<Ref<Position>> q)
// {
//     for (const auto &tup : q) {
//         const auto &pos = std::get<0>(tup);
//
//         if (pos.ptr) {
//             std::cout << " - (" << pos.ptr->x << ", " << pos.ptr->y << ")\n";
//         }
//     }
// }
//
int main()
{
    Application app;

    app.add_system(Schedule::STARTUP, spawn_startup)
        // .add_system(Schedule::UPDATE, setup_systems)
        // .add_system(Schedule::FIXED_UPDATE, print_systems)
        .run();

    return 0;
}
