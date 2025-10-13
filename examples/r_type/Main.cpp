#include <R-Engine/Application.hpp>
#include <R-Engine/Core/FrameTime.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Event.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/Maths/Vec.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>
#include <R-Engine/Plugins/RenderPlugin.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>

#include <cstdlib>
#include <vector>

/* clang-format off */

/* ================================================================================= */
/* Constants & Configuration */
/* ================================================================================= */

static constexpr float PLAYER_SPEED = 3.5f;
static constexpr float BULLET_SPEED = 8.0f;
static constexpr float ENEMY_SPEED = 2.0f;
static constexpr float ENEMY_SPAWN_INTERVAL = 0.75f; /* in seconds */
static constexpr float PLAYER_BOUNDS_PADDING = 0.2f;

/* ================================================================================= */
/* Components */
/* Components are simple data structures that define the properties of an entity. */
/* ================================================================================= */

struct Player {};
struct Enemy {};
struct Bullet {};

struct Velocity {
    r::Vec3f value;
};

struct Collider {
    float radius;
};

/* ================================================================================= */
/* Resources */
/* Resources are global data structures. */
/* ================================================================================= */

struct EnemySpawnTimer {
    float time_left = ENEMY_SPAWN_INTERVAL;
};

/* ================================================================================= */
/* Systems */
/* Systems contain the logic of the game. They query for entities with specific */
/* components and operate on them. */
/* ================================================================================= */

/**
 * @brief (STARTUP) Sets up the game world.
 * @details This system runs once at the beginning. It configures the camera,
 * binds input actions, and spawns the initial player entity.
 */
static void startup_system(
    r::ecs::Commands& commands,
    r::ecs::ResMut<r::Camera3d> camera,
    r::ecs::ResMut<r::InputMap> input_map,
    r::ecs::ResMut<r::Meshes> meshes)
{
    /* --- Configure Camera --- */
    /* Position the camera for a side-scrolling view on the XZ plane (Z is up). */
    camera.ptr->position = {0.0f, -20.0f, 0.0f};
    camera.ptr->target = {0.0f, 0.0f, 0.0f};
    camera.ptr->up = {0.0f, 0.0f, 1.0f};
    camera.ptr->fovy = 45.0f;

    /* --- Bind Inputs --- */
    input_map.ptr->bindAction("MoveUp", r::KEYBOARD, KEY_W);
    input_map.ptr->bindAction("MoveDown", r::KEYBOARD, KEY_S);
    input_map.ptr->bindAction("MoveLeft", r::KEYBOARD, KEY_A);
    input_map.ptr->bindAction("MoveRight", r::KEYBOARD, KEY_D);
    input_map.ptr->bindAction("Fire", r::KEYBOARD, KEY_SPACE);

    /* --- Spawn Player --- */
    ::Mesh player_mesh_data = r::Mesh3d::Cube(1.0f);

    if (player_mesh_data.vertexCount > 0 && player_mesh_data.vertices) {
        r::MeshHandle player_mesh_handle = meshes.ptr->add(player_mesh_data);

        if (player_mesh_handle != r::MeshInvalidHandle) {
            commands.spawn(
                Player{},
                r::Transform3d{
                    .position = {-5.0f, 0.0f, 0.0f},
                    .scale = {0.5f, 0.5f, 0.5f}
                },
                Velocity{{0.0f, 0.0f, 0.0f}},
                Collider{0.5f},
                r::Mesh3d{
                    player_mesh_handle,
                    r::Color{80, 150, 255, 255} /* Blue color for the player */
                }
            );
        } else {
             r::Logger::error("startup_system: Failed to register player mesh with mesh manager.");
        }
    } else {
        r::Logger::error("startup_system: Failed to generate player cube mesh.");
    }
}

/**
 * @brief (UPDATE) Reads player input and updates their velocity.
 * @details Also handles firing bullets when the fire action is pressed.
 */
static void player_input_system(
    r::ecs::Commands& commands,
    r::ecs::Res<r::UserInput> user_input,
    r::ecs::Res<r::InputMap> input_map,
    r::ecs::ResMut<r::Meshes> meshes,
    r::ecs::Query<r::ecs::Mut<Velocity>, r::ecs::Ref<r::Transform3d>, r::ecs::With<Player>> query)
{
    for (auto [velocity, transform, _] : query) {
        /* --- Movement --- */
        r::Vec3f direction = {0.0f, 0.0f, 0.0f};
        if (input_map.ptr->isActionPressed("MoveUp", *user_input.ptr))    direction.z += 1.0f;
        if (input_map.ptr->isActionPressed("MoveDown", *user_input.ptr))  direction.z -= 1.0f;
        if (input_map.ptr->isActionPressed("MoveLeft", *user_input.ptr))  direction.x -= 1.0f;
        if (input_map.ptr->isActionPressed("MoveRight", *user_input.ptr)) direction.x += 1.0f;

        velocity.ptr->value = (direction.length() > 0.0f)
            ? direction.normalize() * PLAYER_SPEED
            : r::Vec3f{0.0f, 0.0f, 0.0f};

        /* --- Firing --- */
        if (input_map.ptr->isActionPressed("Fire", *user_input.ptr)) {
            ::Mesh bullet_mesh_data = r::Mesh3d::Circle(0.5f, 16);
            if (bullet_mesh_data.vertexCount > 0 && bullet_mesh_data.vertices) {
                r::MeshHandle bullet_mesh_handle = meshes.ptr->add(bullet_mesh_data);
                if (bullet_mesh_handle != r::MeshInvalidHandle) {
                    commands.spawn(
                        Bullet{},
                        r::Transform3d{
                            .position = transform.ptr->position + r::Vec3f{0.6f, 0.0f, 0.0f},
                            .scale = {0.2f, 0.2f, 0.2f}
                        },
                        Velocity{{BULLET_SPEED, 0.0f, 0.0f}},
                        Collider{0.2f},
                        r::Mesh3d{
                            bullet_mesh_handle,
                            r::Color{255, 200, 80, 255} /* Yellow color for bullets */
                        }
                    );
                } else {
                    r::Logger::warn("player_input_system: Failed to register bullet mesh.");
                }
            } else {
                r::Logger::warn("player_input_system: Failed to generate bullet circle mesh.");
            }
        }
    }
}

/**
 * @brief (UPDATE) Spawns enemies periodically from the right side of the screen.
 */
static void enemy_spawner_system(
    r::ecs::Commands& commands,
    r::ecs::ResMut<EnemySpawnTimer> spawn_timer,
    r::ecs::Res<r::core::FrameTime> time,
    r::ecs::ResMut<r::Meshes> meshes)
{
    spawn_timer.ptr->time_left -= time.ptr->delta_time;
    if (spawn_timer.ptr->time_left <= 0.0f) {
        spawn_timer.ptr->time_left = ENEMY_SPAWN_INTERVAL;

        /* Spawn a new enemy off-screen to the right at a random height. */
        float random_z = (static_cast<float>(rand()) / RAND_MAX) * 10.0f - 5.0f; /* Range [-5, 5] */

        ::Mesh enemy_mesh_data = r::Mesh3d::Circle(1.0f, 16);
        if (enemy_mesh_data.vertexCount > 0 && enemy_mesh_data.vertices) {
            r::MeshHandle enemy_mesh_handle = meshes.ptr->add(enemy_mesh_data);
            if (enemy_mesh_handle != r::MeshInvalidHandle) {
                commands.spawn(
                    Enemy{},
                    r::Transform3d{
                        .position = {12.0f, 0.0f, random_z},
                        .scale = {0.4f, 0.4f, 0.4f}
                    },
                    Velocity{{-ENEMY_SPEED, 0.0f, 0.0f}},
                    Collider{0.4f},
                    r::Mesh3d{
                        enemy_mesh_handle,
                        r::Color{255, 80, 100, 255} /* Red color for enemies */
                    }
                );
            } else {
                r::Logger::warn("enemy_spawner_system: Failed to register enemy mesh.");
            }
        } else {
            r::Logger::warn("enemy_spawner_system: Failed to generate enemy circle mesh.");
        }
    }
}

/**
 * @brief (UPDATE) Updates the position of all entities with a Velocity.
 */
static void movement_system(
    r::ecs::Query<r::ecs::Mut<r::Transform3d>, r::ecs::Ref<Velocity>> query,
    r::ecs::Res<r::core::FrameTime> time)
{
    for (auto [transform, velocity] : query) {
        transform.ptr->position += velocity.ptr->value * time.ptr->delta_time;
    }
}

/**
 * @brief (UPDATE) Prevents the player from moving off-screen.
 */
static void screen_bounds_system(
    r::ecs::Query<r::ecs::Mut<r::Transform3d>, r::ecs::With<Player>> query)
{
    /* Define screen boundaries based on a 16:9 aspect ratio camera at z=20. */
    const float screen_width_half = 9.0f - PLAYER_BOUNDS_PADDING;
    const float screen_height_half = 5.0f - PLAYER_BOUNDS_PADDING;

    for (auto [transform, _] : query) {
        auto& pos = transform.ptr->position;
        if (pos.x > screen_width_half)  pos.x = screen_width_half;
        if (pos.x < -screen_width_half) pos.x = -screen_width_half;
        if (pos.z > screen_height_half)  pos.z = screen_height_half;
        if (pos.z < -screen_height_half) pos.z = -screen_height_half;
    }
}

/**
 * @brief (UPDATE) Checks for collisions between bullets and enemies.
 * @details If a collision is detected, both entities are despawned.
 */
static void collision_system(
    r::ecs::Commands& commands,
    r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Ref<Collider>, r::ecs::With<Bullet>> bullet_query,
    r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Ref<Collider>, r::ecs::With<Enemy>> enemy_query)
{
    std::vector<r::ecs::Entity> despawn_queue;

    for (auto bullet_it = bullet_query.begin(); bullet_it != bullet_query.end(); ++bullet_it) {
        auto [bullet_transform, bullet_collider, _b] = *bullet_it;

        for (auto enemy_it = enemy_query.begin(); enemy_it != enemy_query.end(); ++enemy_it) {
            auto [enemy_transform, enemy_collider, _e] = *enemy_it;

            float distance = (bullet_transform.ptr->position - enemy_transform.ptr->position).length();
            float radii_sum = bullet_collider.ptr->radius + enemy_collider.ptr->radius;

            if (distance < radii_sum) {
                /* Defer despawning to avoid iterator invalidation. */
                despawn_queue.push_back(bullet_it.entity());
                despawn_queue.push_back(enemy_it.entity());
            }
        }
    }


    for (r::ecs::Entity entity : despawn_queue) {
        commands.despawn(entity);
    }
}

/**
 * @brief (UPDATE) Despawns entities that have moved off-screen.
 */
static void despawn_offscreen_system(
    r::ecs::Commands& commands,
    r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Without<Player>> query)
{
    const float despawn_boundary_x = 13.0f; /* A bit wider than the spawn point */

    for (auto it = query.begin(); it != query.end(); ++it) {
        auto [transform, _] = *it;
        if (std::abs(transform.ptr->position.x) > despawn_boundary_x) {
            commands.despawn(it.entity());
        }
    }
}

int main()
{
    /* Seed random for enemy spawn positions */
    srand(static_cast<unsigned int>(time(nullptr)));

    r::Application{}
        .add_plugins(
            r::DefaultPlugins{}.set(r::WindowPlugin{
                r::WindowPluginConfig {
                    .size = {1280, 720},
                    .title = "R-Type Minimal Prototype",
                }
            })
        )
        .insert_resource(EnemySpawnTimer{})
        .add_systems<startup_system>(r::Schedule::STARTUP)
        .add_systems<
            player_input_system,
            enemy_spawner_system,
            movement_system,
            screen_bounds_system,
            collision_system,
            despawn_offscreen_system
        >(r::Schedule::UPDATE)
        .run();

    return 0;
}

/* clang-format on */
