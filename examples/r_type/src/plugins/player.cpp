#include "player.hpp"
#include "../components.hpp"
#include "../state.hpp"

#include "R-Engine/Components/Transform3d.hpp"
#include <R-Engine/Application.hpp>
#include <R-Engine/Core/FrameTime.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>

// clang-format off

/* ================================================================================= */
/* Constants */
/* ================================================================================= */

static constexpr float PLAYER_SPEED = 3.5f;
static constexpr float BULLET_SPEED = 8.0f;
static constexpr float PLAYER_FIRE_RATE = 0.15f;
static constexpr float PLAYER_BOUNDS_PADDING = 0.2f;

/* ================================================================================= */
/* Run Condition */
/* ================================================================================= */

static bool is_in_gameplay_state(r::ecs::Res<r::State<GameState>> state)
{
    if (!state.ptr) {
        return false;
    }
    auto current_state = state.ptr->current();
    return current_state == GameState::EnemiesBattle || current_state == GameState::BossBattle;
}

/* ================================================================================= */
/* Player Systems */
/* ================================================================================= */

static void spawn_player_system(r::ecs::Commands& commands, r::ecs::ResMut<r::Meshes> meshes)
{
    ::Model player_model_data = r::Mesh3d::Glb("examples/r_type/assets/R-9.glb");
    if (player_model_data.meshCount > 0) {
        r::MeshHandle player_mesh_handle = meshes.ptr->add(player_model_data);

        if (player_mesh_handle != r::MeshInvalidHandle) {
            commands.spawn(
                Player{},
                r::Transform3d{
                    .position = {-5.0f, 0.0f, 0.0f},
                    .rotation = {static_cast<float>(M_PI) / 2.0f, 0.0f, static_cast<float>(M_PI) / 2.0f},
                    .scale = {3.0f, 3.0f, 3.0f}
                },
                Velocity{{0.0f, 0.0f, 0.0f}},
                Collider{0.5f},
                FireCooldown{},
                r::Mesh3d{
                    player_mesh_handle, r::Color{255, 255, 255, 255} /* White tint to show original texture */
                }
            );
        } else {
            r::Logger::error("spawn_player_system: Failed to register player model with mesh manager.");
        }
    } else {
        r::Logger::error("spawn_player_system: Failed to load player model 'assets/R-9.glb'.");
    }
}

static void player_input_system(
    r::ecs::Commands& commands, r::ecs::Res<r::UserInput> user_input, r::ecs::Res<r::InputMap> input_map,
    r::ecs::ResMut<r::Meshes> meshes, r::ecs::Res<r::core::FrameTime> time,
    r::ecs::Query<r::ecs::Mut<Velocity>, r::ecs::Ref<r::Transform3d>, r::ecs::Mut<FireCooldown>, r::ecs::With<Player>> query)
{
    for (auto [velocity, transform, cooldown, _] : query) {
        /* --- Cooldown --- */
        if (cooldown.ptr->timer > 0.0f) {
            cooldown.ptr->timer -= time.ptr->delta_time;
        }

        /* --- Movement --- */
        r::Vec3f direction = {0.0f, 0.0f, 0.0f};
        if (input_map.ptr->isActionPressed("MoveUp", *user_input.ptr)) {
            direction.z += 1.0f;
        }
        if (input_map.ptr->isActionPressed("MoveDown", *user_input.ptr)) {
            direction.z -= 1.0f;
        }
        if (input_map.ptr->isActionPressed("MoveLeft", *user_input.ptr)) {
            direction.x -= 1.0f;
        }
        if (input_map.ptr->isActionPressed("MoveRight", *user_input.ptr)) {
            direction.x += 1.0f;
        }

        velocity.ptr->value = (direction.length() > 0.0f) ? direction.normalize() * PLAYER_SPEED : r::Vec3f{0.0f, 0.0f, 0.0f};

        /* --- Firing --- */
        if (input_map.ptr->isActionPressed("Fire", *user_input.ptr) && cooldown.ptr->timer <= 0.0f) {
            cooldown.ptr->timer = PLAYER_FIRE_RATE;
            ::Mesh bullet_mesh_data = r::Mesh3d::Circle(0.5f, 16);
            if (bullet_mesh_data.vertexCount > 0 && bullet_mesh_data.vertices) {
                r::MeshHandle bullet_mesh_handle = meshes.ptr->add(bullet_mesh_data);
                if (bullet_mesh_handle != r::MeshInvalidHandle) {
                    commands.spawn(
                        PlayerBullet{},
                        r::Transform3d{
                            .position = transform.ptr->position + r::Vec3f{0.6f, 0.0f, 0.0f},
                            .scale = {0.2f, 0.2f, 0.2f}
                        },
                        Velocity{{BULLET_SPEED, 0.0f, 0.0f}},
                        Collider{0.2f},
                        r::Mesh3d{
                            bullet_mesh_handle, r::Color{255, 200, 80, 255} /* Yellow color for bullets */
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

static void screen_bounds_system(r::ecs::Query<r::ecs::Mut<r::Transform3d>, r::ecs::With<Player>> query)
{
    for (auto [transform, _] : query) {
        /* Clamp X position */
        if (transform.ptr->position.x < -8.0f + PLAYER_BOUNDS_PADDING) {
            transform.ptr->position.x = -8.0f + PLAYER_BOUNDS_PADDING;
        }
        if (transform.ptr->position.x > 8.0f - PLAYER_BOUNDS_PADDING) {
            transform.ptr->position.x = 8.0f - PLAYER_BOUNDS_PADDING;
        }

        /* Clamp Z position */
        if (transform.ptr->position.z < -4.5f + PLAYER_BOUNDS_PADDING) {
            transform.ptr->position.z = -4.5f + PLAYER_BOUNDS_PADDING;
        }
        if (transform.ptr->position.z > 4.5f - PLAYER_BOUNDS_PADDING) {
            transform.ptr->position.z = 4.5f - PLAYER_BOUNDS_PADDING;
        }
    }
}

void PlayerPlugin::build(r::Application& app)
{
    app.add_systems<spawn_player_system>(r::OnEnter{GameState::EnemiesBattle})
        .add_systems<player_input_system, screen_bounds_system>(r::Schedule::UPDATE)
        .run_if<is_in_gameplay_state>();
}
// clang-format on
