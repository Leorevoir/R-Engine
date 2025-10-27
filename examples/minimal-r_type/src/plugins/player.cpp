#include "player.hpp"
#include "../components.hpp"
#include "../resources.hpp"
#include "../state.hpp"

#include "R-Engine/Components/Transform3d.hpp"
#include <R-Engine/Application.hpp>
#include <R-Engine/Core/FrameTime.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/ECS/RunConditions.hpp>
#include <R-Engine/Maths/Maths.hpp>
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>
#include <R-Engine/Plugins/RenderPlugin.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>
#include <algorithm>
#include <cmath>
#include <utility>

// clang-format off

/* ================================================================================= */
/* Constants */
/* ================================================================================= */

static constexpr float PLAYER_SPEED = 3.5f;
static constexpr float BULLET_SPEED = 8.0f;
static constexpr float PLAYER_FIRE_RATE = 0.15f;
static constexpr float PLAYER_BOUNDS_PADDING = 0.5f;

/* ================================================================================= */
/* Player Systems */
/* ================================================================================= */

static void setup_bullet_assets_system(r::ecs::Commands& commands, r::ecs::ResMut<r::Meshes> meshes)
{
    PlayerBulletAssets bullet_assets;

    bullet_assets.laser_beam_handle = meshes.ptr->add("examples/minimal-r_type/assets/models/PlayerMissile.glb");
    if (bullet_assets.laser_beam_handle == r::MeshInvalidHandle) {
        r::Logger::error("Failed to queue player missile model!");
    }

    commands.insert_resource(bullet_assets);
}

static void spawn_player_system(r::ecs::Commands& commands, r::ecs::ResMut<r::Meshes> meshes)
{
    r::MeshHandle player_mesh_handle = meshes.ptr->add("examples/minimal-r_type/assets/models/R-9.glb");

    if (player_mesh_handle != r::MeshInvalidHandle) {
        commands.spawn(
            Player{},
            r::Transform3d{
                .position = {-5.0f, 0.0f, 0.0f},
                .scale = {3.0f, 3.0f, 3.0f}
            },
            Velocity{{0.0f, 0.0f, 0.0f}},
            Collider{0.5f},
            FireCooldown{},
            r::Mesh3d{
                .id = player_mesh_handle,
                .color = r::Color{255, 255, 255, 255},
                .rotation_offset = {0.0f, static_cast<float>(M_PI) / 2.0f, 0.0f}
            }
        );
    } else {
        r::Logger::error("Failed to queue player model for loading: examples/minimal-r_type/assets/models/R-9.glb");
    }
}

static void player_input_system(
    r::ecs::Commands& commands, r::ecs::Res<r::UserInput> user_input, r::ecs::Res<r::InputMap> input_map,
    r::ecs::Res<PlayerBulletAssets> bullet_assets, r::ecs::Res<r::core::FrameTime> time,
    r::ecs::Query<r::ecs::Mut<Velocity>, r::ecs::Ref<r::Transform3d>, r::ecs::Mut<FireCooldown>, r::ecs::With<Player>> query)
{
    for (auto [velocity, transform, cooldown, _] : query) {
        /* --- Cooldown --- */
        if (cooldown.ptr->timer > 0.0f) {
            cooldown.ptr->timer -= time.ptr->delta_time;
        }

        /* --- Movement --- */
        r::Vec3f direction = {0.0f, 0.0f, 0.0f};
        if (input_map.ptr->isActionPressed("MoveUp", *user_input.ptr))
            direction.y += 1.0f;
        if (input_map.ptr->isActionPressed("MoveDown", *user_input.ptr))
            direction.y -= 1.0f;
        if (input_map.ptr->isActionPressed("MoveLeft", *user_input.ptr))
            direction.x -= 1.0f;
        if (input_map.ptr->isActionPressed("MoveRight", *user_input.ptr))
            direction.x += 1.0f;

        velocity.ptr->value = (direction.length() > 0.0f) ? direction.normalize() * PLAYER_SPEED : r::Vec3f{0.0f, 0.0f, 0.0f};

        /* --- Firing --- */
        if (input_map.ptr->isActionPressed("Fire", *user_input.ptr) && cooldown.ptr->timer <= 0.0f) {
            cooldown.ptr->timer = PLAYER_FIRE_RATE;

            if (bullet_assets.ptr->laser_beam_handle != r::MeshInvalidHandle) {
                commands.spawn(
                    PlayerBullet{},
                    r::Transform3d{
                        .position = transform.ptr->position + r::Vec3f{0.6f, 0.0f, 0.0f},
                        .scale = {0.2f, 0.2f, 0.2f}
                    },
                    Velocity{{BULLET_SPEED, 0.0f, 0.0f}},
                    Collider{0.2f},
                    r::Mesh3d{
                        .id = bullet_assets.ptr->laser_beam_handle,
                        .color = r::Color{255, 255, 255, 255},
                        .rotation_offset = {-(static_cast<float>(M_PI) / 2.0f), 0.0f, -static_cast<float>(M_PI) / 2.0f}
                    }
                );
            }
        }
    }
}

static void screen_bounds_system(r::ecs::Query<r::ecs::Mut<r::Transform3d>, r::ecs::With<Player>> query, r::ecs::Res<r::Camera3d> camera,
    r::ecs::Res<r::WindowPluginConfig> window_config)
{
    if (!camera.ptr || !window_config.ptr) {
        return;
    }

    const float distance = camera.ptr->position.z;
    const float aspect_ratio = static_cast<float>(window_config.ptr->size.width) / static_cast<float>(window_config.ptr->size.height);
    const float fovy_rad = camera.ptr->fovy * (r::R_PI / 180.0f);

    const float view_height = 2.0f * distance * std::tanf(fovy_rad / 2.0f);
    const float view_width = view_height * aspect_ratio;

    const float half_height = view_height / 2.0f;
    const float half_width = view_width / 2.0f;

    for (auto [transform, _] : query) {
        transform.ptr->position.x =
            std::clamp(transform.ptr->position.x, -half_width + PLAYER_BOUNDS_PADDING, half_width - PLAYER_BOUNDS_PADDING);
        transform.ptr->position.y =
            std::clamp(transform.ptr->position.y, -half_height + PLAYER_BOUNDS_PADDING, half_height - PLAYER_BOUNDS_PADDING);
    }
}


void PlayerPlugin::build(r::Application& app)
{
    app.add_systems<setup_bullet_assets_system>(r::OnEnter{GameState::EnemiesBattle})
        .add_systems<spawn_player_system>(r::OnEnter{GameState::EnemiesBattle})
        .add_systems<player_input_system, screen_bounds_system>(r::Schedule::UPDATE)
        .run_if<r::run_conditions::in_state<GameState::EnemiesBattle>>()
        .run_or<r::run_conditions::in_state<GameState::BossBattle>>();
}
// clang-format on
