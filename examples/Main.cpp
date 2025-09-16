#include <R-Engine/Application.hpp>
#include <R-Engine/ECS/Command.hpp>

#include <iostream>

struct Position {
        f32 x, y = 0;
};

struct Velocity {
        f32 vx = 0, vy = 0;
};

struct Health {
        int value = 100;
};

struct PlayerTag {
};

using namespace r;
using namespace ecs;

static void spawn_startup(Commands cmds)
{
    std::cout << "--- Spawning entities ---\n";
    // Entity 1: Has Position, Velocity, Health, and PlayerTag
    const Entity e1 = cmds.spawn();
    cmds.add_component<Position>(e1, Position{0.0f, 0.0f});
    cmds.add_component<Velocity>(e1, Velocity{1.0f, 0.5f});
    cmds.add_component<Health>(e1, Health{100});
    cmds.add_component<PlayerTag>(e1, {});
    std::cout << "Spawned Entity 1 with Position, Velocity, Health, PlayerTag\n";

    // Entity 2: Has Position and Velocity
    const Entity e2 = cmds.spawn();
    cmds.add_component<Position>(e2, Position{10.0f, -5.0f});
    cmds.add_component<Velocity>(e2, Velocity{-0.2f, 0.0f});
    std::cout << "Spawned Entity 2 with Position, Velocity\n";

    // Entity 3: Has only Position and Health
    const Entity e3 = cmds.spawn();
    cmds.add_component<Position>(e3, Position{5.0f, 5.0f});
    cmds.add_component<Health>(e3, Health{50});
    std::cout << "Spawned Entity 3 with Position, Health\n";
    std::cout << "-------------------------\n\n";
}

// System that moves entities with Position and Velocity
static void movement_system(Res<core::FrameTime> time, Query<Mut<Position>, Ref<Velocity>> q)
{
    for (auto [pos_mut, vel_ref] : q) {
        pos_mut.ptr->x += vel_ref.ptr->vx * time.ptr->delta_time;
        pos_mut.ptr->y += vel_ref.ptr->vy * time.ptr->delta_time;
    }
}

// System that prints positions of all entities
static void print_positions_system(Query<Ref<Position>> q)
{
    std::cout << "--- All positions ---\n";
    for (auto [pos] : q) {
        std::cout << " - (" << pos.ptr->x << ", " << pos.ptr->y << ")\n";
    }
}

// Example 1: Use With<> and Without<>
// Finds entities that are players (With<PlayerTag>) but are not moving (Without<Velocity>)
// This query should not find any entity in the setup.
static void find_idle_players_system(Query<Ref<Position>, With<PlayerTag>, Without<Velocity>> q)
{
    std::cout << "\n--- Idle Players (With<PlayerTag>, Without<Velocity>) ---\n";
    bool found = false;
    for (auto [pos, player_tag, no_velocity] : q) {
        std::cout << "Found idle player at: (" << pos.ptr->x << ", " << pos.ptr->y << ")\n";
        found = true;
    }
    if (!found) {
        std::cout << "No idle players found.\n";
    }
}

// Example 2: Use Without<>
// Finds entities with Position that have NO Health component
// Should find Entity 2.
static void find_entities_without_health_system(Query<Ref<Position>, Without<Health>> q)
{
    std::cout << "\n--- Entities without Health ---\n";
    for (auto [pos, _] : q) {
        std::cout << "Found entity without health at: (" << pos.ptr->x << ", " << pos.ptr->y << ")\n";
    }
}

// Example 3: Use Optional<>
// Finds all entities with Position and checks if they optionally have Health
// Should find all 3 entities and print their health status.
static void check_health_system(Query<Ref<Position>, Optional<Health>> q)
{
    std::cout << "\n--- Checking Health Status (with Optional<Health>) ---\n";
    for (auto [pos, health_opt] : q) {
        if (health_opt.ptr) {
            std::cout << "Entity at (" << pos.ptr->x << ", " << pos.ptr->y << ") has health: " << health_opt.ptr->value << "\n";
        } else {
            std::cout << "Entity at (" << pos.ptr->x << ", " << pos.ptr->y << ") has no health component.\n";
        }
    }
}

int main()
{
    Application app;

    app.add_system(Schedule::STARTUP, spawn_startup)
        .add_system(Schedule::UPDATE, movement_system)
        // Add example systems to the update schedule
        .add_system(Schedule::UPDATE, find_idle_players_system)
        .add_system(Schedule::UPDATE, find_entities_without_health_system)
        .add_system(Schedule::UPDATE, check_health_system)
        // Print positions at the end of the frame
        .add_system(Schedule::UPDATE, print_positions_system)
        .run();

    return 0;
}
