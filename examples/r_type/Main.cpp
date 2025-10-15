#include "R-Engine/Core/States.hpp"
#include "R-Engine/ECS/RunConditions.hpp"
#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Core/FrameTime.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Core/States.hpp>
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Event.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/ECS/RunConditions.hpp>
#include <R-Engine/Maths/Vec.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>
#include <R-Engine/Plugins/RenderPlugin.hpp>
#include <R-Engine/Plugins/Ui/Systems.hpp>
#include <R-Engine/Plugins/UiPlugin.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>
#include <R-Engine/UI/Button.hpp>
#include <R-Engine/UI/Components.hpp>
#include <R-Engine/UI/Events.hpp>
#include <R-Engine/UI/Image.hpp>
#include <R-Engine/UI/InputState.hpp>
#include <R-Engine/UI/Text.hpp>
#include <R-Engine/UI/Theme.hpp>

#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>

/* clang-format off */

/* ================================================================================= */
/* Game State */
/* ================================================================================= */

enum class GameState {
    MainMenu,
    GameOver,
    EnemiesBattle,
    BossBattle
};

/* ================================================================================= */
/* Constants & Configuration */
/* ================================================================================= */

static constexpr float PLAYER_SPEED = 3.5f;
static constexpr float BULLET_SPEED = 8.0f;
static constexpr float ENEMY_SPEED = 2.0f;
static constexpr float ENEMY_SPAWN_INTERVAL = 0.75f; /* in seconds */
static constexpr float PLAYER_FIRE_RATE = 0.15f;
static constexpr float PLAYER_BOUNDS_PADDING = 0.2f;
static constexpr float BOSS_SPAWN_TIME = 20.0f;
static constexpr float BOSS_VERTICAL_SPEED = 4.5f;
static constexpr float BOSS_UPPER_BOUND = 4.0f;
static constexpr float BOSS_LOWER_BOUND = -15.0f;

/* ================================================================================= */
/* Menu Components */
/* ================================================================================= */

struct MenuButton {
    enum class Action {
        None,
        Play,
        Options,
        Quit
    };
    Action action = Action::None;
};

struct MenuRoot {};

/* ================================================================================= */
/* Game Components */
/* Components are simple data structures that define the properties of an entity. */
/* ================================================================================= */

struct Player {};
struct Boss {};
struct Enemy {};
struct PlayerBullet {};
struct EnemyBullet {};

struct FireCooldown {
    float timer = 0.0f;
};

struct Velocity {
    r::Vec3f value;
};

struct Collider {
    float radius;
};

struct Health {
    int current;
    int max;
};

/* ================================================================================= */
/* Resources */
/* Resources are global data structures. */
/* ================================================================================= */

struct EnemySpawnTimer {
    float time_left = ENEMY_SPAWN_INTERVAL;
};

struct BossSpawnTimer {
    float time_left = BOSS_SPAWN_TIME;
    bool spawned = false;
};

struct BossShootTimer {
    float time_left = 2.0f;
    static constexpr float FIRE_RATE = 2.0f;
};

/* ================================================================================= */
/* Menu Systems */
/* ================================================================================= */

static void setup_ui_theme(r::ecs::ResMut<r::UiTheme> theme, r::ecs::ResMut<r::UiPluginConfig> cfg)
{
    cfg.ptr->show_debug_overlay = false;

    /* R-Type cyan theme (#62DDFF) */
    theme.ptr->button.bg_normal = r::Color{0, 36, 48, 255};
    theme.ptr->button.bg_hover = r::Color{98, 221, 255, 100};
    theme.ptr->button.bg_pressed = r::Color{98, 221, 255, 150};
    theme.ptr->button.bg_disabled = r::Color{50, 50, 50, 255};

    theme.ptr->button.border_normal = r::Color{98, 221, 255, 255};
    theme.ptr->button.border_hover = r::Color{98, 221, 255, 255};
    theme.ptr->button.border_pressed = r::Color{98, 221, 255, 255};
    theme.ptr->button.border_disabled = r::Color{100, 100, 100, 255};

    theme.ptr->button.border_thickness = 2.f;
    theme.ptr->button.text = r::Color{98, 221, 255, 255};
}

static void build_main_menu(r::ecs::Commands &cmds)
{
    r::Logger::info("Building main menu");

    /* Root menu container */
    auto menu_root = cmds.spawn(
        MenuRoot{},
        r::UiNode{},
        r::Style{
            .width_pct = 100.f,
            .height_pct = 100.f,
            .background = r::Color{0, 0, 0, 255},
            .margin = 0.f,
            .padding = 0.f,
            .direction = r::LayoutDirection::Column,
            .justify = r::JustifyContent::Center,
            .align = r::AlignItems::Center,
            .gap = 10.f
        },
        r::ComputedLayout{},
        r::Visibility::Visible
    );

    menu_root.with_children([&](r::ecs::ChildBuilder &parent) {
        /* R-Type Title Logo */
        parent.spawn(
            r::UiNode{},
            r::Style{
                .height = 200.f,
                .width_pct = 100.f,
                .background = r::Color{0, 0, 0, 1},
                .margin = 0.f,
                .padding = 0.f
            },
            r::UiImage{
                .path = "assets/r-type_title.png",
                .tint = r::Color{255, 255, 255, 255},
                .keep_aspect = true
            },
            r::ComputedLayout{},
            r::Visibility::Visible
        );

        /* Play Button */
        parent.spawn(
            r::UiNode{},
            r::UiButton{},
            MenuButton{MenuButton::Action::Play},
            r::Style{
                .width = 280.f,
                .height = 45.f,
                .direction = r::LayoutDirection::Column,
                .justify = r::JustifyContent::Center,
                .align = r::AlignItems::Center
            },
            r::UiText{
                .content = std::string("Play"),
                .font_size = 22
            },
            r::ComputedLayout{},
            r::Visibility::Visible
        );

        /* Options Button */
        parent.spawn(
            r::UiNode{},
            r::UiButton{},
            MenuButton{MenuButton::Action::Options},
            r::Style{
                .width = 280.f,
                .height = 45.f,
                .direction = r::LayoutDirection::Column,
                .justify = r::JustifyContent::Center,
                .align = r::AlignItems::Center
            },
            r::UiText{
                .content = std::string("Options"),
                .font_size = 22
            },
            r::ComputedLayout{},
            r::Visibility::Visible
        );

        /* Quit Button */
        parent.spawn(
            r::UiNode{},
            r::UiButton{},
            MenuButton{MenuButton::Action::Quit},
            r::Style{
                .width = 280.f,
                .height = 45.f,
                .direction = r::LayoutDirection::Column,
                .justify = r::JustifyContent::Center,
                .align = r::AlignItems::Center
            },
            r::UiText{
                .content = std::string("Quit"),
                .font_size = 22
            },
            r::ComputedLayout{},
            r::Visibility::Visible
        );
    });
}

static void menu_button_handler(
    r::ecs::Res<r::UiInputState> input_state,
    r::ecs::Query<r::ecs::Ref<MenuButton>> buttons,
    r::ecs::ResMut<r::NextState<GameState>> next_state
)
{
    const auto clicked = input_state.ptr->last_clicked;
    if (clicked == r::ecs::NULL_ENTITY) {
        return;
    }

    MenuButton::Action action = MenuButton::Action::None;
    for (auto it = buttons.begin(); it != buttons.end(); ++it) {
        auto [btn] = *it;
        if (static_cast<r::ecs::Entity>(it.entity()) == clicked && btn.ptr) {
            action = btn.ptr->action;
            break;
        }
    }

    switch (action) {
        case MenuButton::Action::Play:
            r::Logger::info("Starting game...");
            next_state.ptr->set(GameState::EnemiesBattle);
            break;
        case MenuButton::Action::Options:
            r::Logger::info("Options clicked (not implemented)");
            break;
        case MenuButton::Action::Quit:
            r::Logger::info("Quitting game...");
            r::Application::quit.store(true, std::memory_order_relaxed);
            break;
        default:
            break;
    }
}

static void cleanup_menu(
    r::ecs::Commands &cmds,
    r::ecs::Query<r::ecs::Ref<MenuRoot>> menu_entities
)
{
    r::Logger::info("Cleaning up menu");
    for (auto it = menu_entities.begin(); it != menu_entities.end(); ++it) {
        cmds.despawn(it.entity());
    }
}

/* ================================================================================= */
/* Game Systems */
/* Systems contain the logic of the game. They query for entities with specific */
/* components and operate on them. */
/* ================================================================================= */

/**
 * @brief (STARTUP) Sets up the game world.
 * @details This system runs once at the beginning. It configures the camera,
 * binds input actions, and spawns the initial player entity.
 */
static void startup_system(
    r::ecs::Commands& commands, r::ecs::ResMut<r::Camera3d> camera,
    r::ecs::ResMut<r::InputMap> input_map, r::ecs::ResMut<r::Meshes> meshes)
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
    ::Model player_model_data = r::Mesh3d::Glb("examples/r_type/assets/R-9.glb");
    if (player_model_data.meshCount > 0) {
        r::MeshHandle player_mesh_handle = meshes.ptr->add(player_model_data);

        if (player_mesh_handle != r::MeshInvalidHandle) {
            commands.spawn(
                Player{},
                r::Transform3d{
                    .position = {-5.0f, 0.0f, 0.0f},
                    .rotation = {static_cast<float>(M_PI) / 2.0f, 0.0f,
                                 static_cast<float>(M_PI) / 2.0f},
                    .scale = {3.0f, 3.0f, 3.0f}},
                Velocity{{0.0f, 0.0f, 0.0f}}, Collider{0.5f}, FireCooldown{},
                r::Mesh3d{
                    player_mesh_handle,
                    r::Color{255, 255, 255,
                             255} /* White tint to show original texture */
                });
        } else {
            r::Logger::error(
                "startup_system: Failed to register player model with mesh "
                "manager.");
        }
    } else {
        r::Logger::error(
            "startup_system: Failed to load player model 'assets/R-9.glb'.");
    }
}

/**
 * @brief Makes the boss spawn
 */
static void boss_spawn_system(
    r::ecs::Commands& commands,
    r::ecs::Res<r::core::FrameTime> time,
    r::ecs::ResMut<BossSpawnTimer> spawn_timer,
    r::ecs::ResMut<r::Meshes> meshes)
{
    ::Model boss_model_data = r::Mesh3d::Glb("examples/r_type/assets/Boss.glb");

    if (boss_model_data.meshCount > 0) {
        r::MeshHandle boss_mesh_handle = meshes.ptr->add(boss_model_data);
        if (boss_mesh_handle != r::MeshInvalidHandle) {
            commands.spawn(
                Boss{},
                BossShootTimer{},
                Health{400, 400},
                r::Transform3d{
                    .position = {24.0f, 35.0f, -10.0f},
                    .rotation = {static_cast<float>(M_PI) / 2.0f, 0.0f, -static_cast<float>(M_PI) / 2.0f},
                    .scale = {1.0f, 1.0f, 1.0f}
                },
                Velocity{{0.0f, 0.0f, BOSS_VERTICAL_SPEED}},
                Collider{35.0f},
                r::Mesh3d{
                    boss_mesh_handle,
                    r::Color{255, 255, 255, 255} /* White tint to show original texture */
                }
            );
        } else {
             r::Logger::error("startup_system: Failed to register boss model with mesh manager.");
        }
    } else {
        r::Logger::error("startup_system: Failed to load boss model 'assets/Boss.glb'.");
    }
}


/**
 * @brief Boss **ai** system and movements.
 */
static void boss_ai_system(
    r::ecs::Commands& commands,
    r::ecs::Res<r::core::FrameTime> time,
    r::ecs::ResMut<r::Meshes> meshes,
    r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Mut<Velocity>, r::ecs::Mut<BossShootTimer>, r::ecs::Ref<Health>, r::ecs::With<Boss>> query)
{
    for (auto [transform, velocity, timer, health, _] : query) {
        if (transform.ptr->position.z > BOSS_UPPER_BOUND && velocity.ptr->value.z > 0) {
            velocity.ptr->value.z *= -1;
        } else if (transform.ptr->position.z < BOSS_LOWER_BOUND && velocity.ptr->value.z < 0) {
            velocity.ptr->value.z *= -1;
        }

        timer.ptr->time_left -= time.ptr->delta_time;

        if (timer.ptr->time_left <= 0.0f) {
            timer.ptr->time_left = BossShootTimer::FIRE_RATE;

            ::Mesh bullet_mesh_data = r::Mesh3d::Circle(2.0f, 16);
            if (bullet_mesh_data.vertexCount > 0 && bullet_mesh_data.vertices) {
                r::MeshHandle bullet_mesh_handle = meshes.ptr->add(bullet_mesh_data);
                if (bullet_mesh_handle != r::MeshInvalidHandle) {
                    commands.spawn(
                        EnemyBullet{},
                        r::Transform3d{
                            .position = transform.ptr->position - r::Vec3f{1.6f, 0.0f, 0.0f},
                            .scale = {0.3f, 0.3f, 0.3f}
                        },
                        Velocity{{-BULLET_SPEED, 0.0f, 0.0f}},
                        Collider{0.3f},
                        r::Mesh3d{
                            bullet_mesh_handle,
                            r::Color{255, 80, 220, 255}
                        }
                    );
                    if (health.ptr->current <= health.ptr->max / 2) {
                        commands.spawn(
                            EnemyBullet{},
                            r::Transform3d{
                                .position = transform.ptr->position + r::Vec3f{0.0f, 0.0f, 5.5f},
                                .scale = {0.6f, 0.6f, 0.6f}
                            },
                            Velocity{{-BULLET_SPEED, 0.0f, 0.0f}},
                            Collider{0.3f},
                            r::Mesh3d{
                                bullet_mesh_handle,
                                r::Color{255, 150, 50, 255}
                            }
                        );
                    }
                }
            }
        }
    }
}

static void setup_boss_fight_system(r::ecs::ResMut<r::NextState<GameState>> next_state,
    r::ecs::Res<r::core::FrameTime> time,
    r::ecs::ResMut<BossSpawnTimer> spawn_timer)
{
    if (spawn_timer.ptr->spawned) {
        return;
    }
    spawn_timer.ptr->time_left -= time.ptr->delta_time;
    if (spawn_timer.ptr->time_left <= 0.0f) {
        spawn_timer.ptr->spawned = true;
        next_state.ptr->set(GameState::BossBattle);
    }
}

/**
 * @brief (UPDATE) Reads player input and updates their velocity.
 * @details Also handles firing bullets when the fire action is pressed.
 */
static void player_input_system(
    r::ecs::Commands& commands, r::ecs::Res<r::UserInput> user_input,
    r::ecs::Res<r::InputMap> input_map, r::ecs::ResMut<r::Meshes> meshes,
    r::ecs::Res<r::core::FrameTime> time,
    r::ecs::Query<r::ecs::Mut<Velocity>, r::ecs::Ref<r::Transform3d>,
                  r::ecs::Mut<FireCooldown>, r::ecs::With<Player>>
        query)
{
    for (auto [velocity, transform, cooldown, _] : query) {
        /* --- Cooldown --- */
        if (cooldown.ptr->timer > 0.0f) {
            cooldown.ptr->timer -= time.ptr->delta_time;
        }

        /* --- Movement --- */
        r::Vec3f direction = {0.0f, 0.0f, 0.0f};
        if (input_map.ptr->isActionPressed("MoveUp", *user_input.ptr))
            direction.z += 1.0f;
        if (input_map.ptr->isActionPressed("MoveDown", *user_input.ptr))
            direction.z -= 1.0f;
        if (input_map.ptr->isActionPressed("MoveLeft", *user_input.ptr))
            direction.x -= 1.0f;
        if (input_map.ptr->isActionPressed("MoveRight", *user_input.ptr))
            direction.x += 1.0f;

        velocity.ptr->value = (direction.length() > 0.0f)
                                  ? direction.normalize() * PLAYER_SPEED
                                  : r::Vec3f{0.0f, 0.0f, 0.0f};

        /* --- Firing --- */
        if (input_map.ptr->isActionPressed("Fire", *user_input.ptr) &&
            cooldown.ptr->timer <= 0.0f) {
            cooldown.ptr->timer = PLAYER_FIRE_RATE;
            ::Mesh bullet_mesh_data = r::Mesh3d::Circle(0.5f, 16);
            if (bullet_mesh_data.vertexCount > 0 && bullet_mesh_data.vertices) {
                r::MeshHandle bullet_mesh_handle =
                    meshes.ptr->add(bullet_mesh_data);
                if (bullet_mesh_handle != r::MeshInvalidHandle) {
                    commands.spawn(
                        PlayerBullet{},
                        r::Transform3d{
                            .position = transform.ptr->position +
                                        r::Vec3f{0.6f, 0.0f, 0.0f},
                            .scale = {0.2f, 0.2f, 0.2f}},
                        Velocity{{BULLET_SPEED, 0.0f, 0.0f}}, Collider{0.2f},
                        r::Mesh3d{bullet_mesh_handle,
                                  r::Color{255, 200, 80,
                                           255} /* Yellow color for bullets */
                        });
                } else {
                    r::Logger::warn(
                        "player_input_system: Failed to register bullet mesh.");
                }
            } else {
                r::Logger::warn(
                    "player_input_system: Failed to generate bullet circle "
                    "mesh.");
            }
        }
    }
}

/**
 * @brief (UPDATE) Spawns enemies periodically from the right side of the
 * screen.
 */
static void enemy_spawner_system(
    r::ecs::Commands& commands, r::ecs::ResMut<EnemySpawnTimer> spawn_timer,
    r::ecs::Res<r::core::FrameTime> time, r::ecs::ResMut<r::Meshes> meshes)
{
    spawn_timer.ptr->time_left -= time.ptr->delta_time;
    if (spawn_timer.ptr->time_left <= 0.0f) {
        spawn_timer.ptr->time_left = ENEMY_SPAWN_INTERVAL;

        /* Spawn enemy at random Z position */
        float random_z = (static_cast<float>(rand()) / RAND_MAX) * 10.0f - 5.0f;

        ::Model enemy_model_data = r::Mesh3d::Glb("examples/r_type/assets/enemy.glb");
        if (enemy_model_data.meshCount > 0) {
            r::MeshHandle enemy_mesh_handle = meshes.ptr->add(enemy_model_data);
            if (enemy_mesh_handle != r::MeshInvalidHandle) {
                commands.spawn(
                    Enemy{},
                    r::Transform3d{
                        .position = {15.0f, 0.0f, random_z},
                        .rotation = {static_cast<float>(M_PI) / 2.0f, 0.0f,
                                     -static_cast<float>(M_PI) / 2.0f},
                        .scale = {1.0f, 1.0f, 1.0f}},
                    Velocity{{-ENEMY_SPEED, 0.0f, 0.0f}}, Collider{0.5f},
                    r::Mesh3d{enemy_mesh_handle, r::Color{255, 255, 255, 255}});
            } else {
                r::Logger::warn(
                    "enemy_spawner_system: Failed to register enemy mesh.");
            }
        } else {
            r::Logger::warn(
                "enemy_spawner_system: Failed to load enemy model.");
        }
    }
}

/**
 * @brief (UPDATE) Applies velocity to position for all entities with both
 * components.
 */
static void movement_system(
    r::ecs::Res<r::core::FrameTime> time,
    r::ecs::Query<r::ecs::Mut<r::Transform3d>, r::ecs::Ref<Velocity>> query)
{
    for (auto [transform, velocity] : query) {
        transform.ptr->position =
            transform.ptr->position + velocity.ptr->value * time.ptr->delta_time;
    }
}

/**
 * @brief (UPDATE) Keeps the player within screen bounds.
 */
static void screen_bounds_system(
    r::ecs::Query<r::ecs::Mut<r::Transform3d>, r::ecs::With<Player>> query)
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

/**
 * @brief (UPDATE) Checks for bullet-enemy collisions and despawns both.
 */
static void collision_system(
    r::ecs::Commands& commands,
    r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Ref<Collider>, r::ecs::With<PlayerBullet>> bullet_query,
    r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Ref<Collider>, r::ecs::With<Enemy>> enemy_query,
    r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Ref<Collider>, r::ecs::Mut<Health>, r::ecs::With<Boss>> boss_query)
{
    std::vector<r::ecs::Entity> despawn_queue;

    for (auto bullet_it = bullet_query.begin(); bullet_it != bullet_query.end(); ++bullet_it) {
        auto [bullet_transform, bullet_collider, _b] = *bullet_it;
        bool bullet_collided = false;

        // Collision avec les ennemis
        for (auto enemy_it = enemy_query.begin(); enemy_it != enemy_query.end(); ++enemy_it) {
            auto [enemy_transform, enemy_collider, _e] = *enemy_it;
            float distance = (bullet_transform.ptr->position - enemy_transform.ptr->position).length();
            float radii_sum = bullet_collider.ptr->radius + enemy_collider.ptr->radius;

            if (distance < radii_sum) {
                despawn_queue.push_back(enemy_it.entity());
                bullet_collided = true;
                break;
            }
        }

        if (bullet_collided) {
            despawn_queue.push_back(bullet_it.entity());
            continue;
        }

        // Collision avec le boss
        for (auto boss_it = boss_query.begin(); boss_it != boss_query.end(); ++boss_it) {
            auto [boss_transform, boss_collider, health, _boss] = *boss_it;
            float distance = (bullet_transform.ptr->position - boss_transform.ptr->position).length();
            float radii_sum = bullet_collider.ptr->radius + boss_collider.ptr->radius;

            if (distance < radii_sum) {
                despawn_queue.push_back(bullet_it.entity());
                health.ptr->current -= 10;
                std::cout << "Boss hit! HP: " << health.ptr->current << health.ptr->max << std::endl;

                if (health.ptr->current <= 0) {
                    despawn_queue.push_back(boss_it.entity());
                    r::Logger::info("Boss defeated!");
                }
                break;
            }
        }
    }

    for (r::ecs::Entity entity : despawn_queue) {
        commands.despawn(entity);
    }
}
/**
 * @brief (UPDATE) Checks for player-enemy collisions and transitions to
 * GameOver.
 */
static void player_collision_system(
    r::ecs::ResMut<r::NextState<GameState>> next_state,
    r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Ref<Collider>,
                  r::ecs::With<Player>>
        player_query,
    r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Ref<Collider>,
                  r::ecs::With<Enemy>>
        enemy_query)
{
    for (auto [player_transform, player_collider, _p] : player_query) {
        for (auto [enemy_transform, enemy_collider, _e] : enemy_query) {
            r::Vec3f delta = player_transform.ptr->position -
                             enemy_transform.ptr->position;
            float distance_squared = delta.x * delta.x + delta.z * delta.z;

            float sum_radii =
                player_collider.ptr->radius + enemy_collider.ptr->radius;
            if (distance_squared < sum_radii * sum_radii) {
                r::Logger::warn("Player collision! Game Over.");
                next_state.ptr->set(GameState::GameOver);
                return;
            }
        }
    }
}

/**
 * @brief (UPDATE) Checks for player-enemy_bullet collisions and transitions to GameOver.
 */
static void player_bullet_collision_system(
    r::ecs::ResMut<r::NextState<GameState>> next_state,
    r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Ref<Collider>, r::ecs::With<Player>> player_query,
    r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Ref<Collider>, r::ecs::With<EnemyBullet>> bullet_query)
{
    for (auto [player_transform, player_collider, _p] : player_query) {
        for (auto [bullet_transform, bullet_collider, _b] : bullet_query) {
            r::Vec3f delta = player_transform.ptr->position - bullet_transform.ptr->position;
            float distance_squared = delta.x * delta.x + delta.z * delta.z;

            float sum_radii = player_collider.ptr->radius + bullet_collider.ptr->radius;
            if (distance_squared < sum_radii * sum_radii) {
                r::Logger::warn("Player hit by bullet! Game Over.");
                next_state.ptr->set(GameState::GameOver);
                return;
            }
        }
    }
}

/**
 * @brief (UPDATE) Despawns entities (bullets, enemies) that go off-screen.
 */
static void despawn_offscreen_system(
    r::ecs::Commands& commands,
    r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Without<Player>, r::ecs::Without<Boss>> query)
{
    const float despawn_boundary_x = 100.0f;
    for (auto it = query.begin(); it != query.end(); ++it) {
        auto [transform, _, __] = *it;
        if (std::abs(transform.ptr->position.x) > despawn_boundary_x) {
            commands.despawn(it.entity());
        }
    }
}

/**
 * @brief (OnEnter: GameOver) Displays a game over message.
 */
static void game_over_message_system()
{
    r::Logger::info("=== GAME OVER ===");
    r::Logger::info("Press ESC to quit.");
}

/**
 * @brief (UPDATE in GameOver) Allows quitting with ESC.
 */
static void game_over_system(r::ecs::Res<r::UserInput> user_input,
                              r::ecs::ResMut<r::NextState<GameState>> next_state)
{
    if (user_input.ptr->isKeyPressed(KEY_ESCAPE)) {
        r::Application::quit.store(true, std::memory_order_relaxed);
    }

    if (user_input.ptr->isKeyPressed(KEY_ENTER)) {
        r::Logger::info("Restarting game...");
        next_state.ptr->set(GameState::EnemiesBattle);
    }
}

/**
 * @brief (OnEnter: InGame) Cleans up entities from the previous game and resets
 * the player.
 */
static void cleanup_system(
    r::ecs::Commands& commands, r::ecs::ResMut<EnemySpawnTimer> spawn_timer,
    r::ecs::Query<r::ecs::With<Enemy>> enemy_query,
    r::ecs::Query<r::ecs::With<PlayerBullet>> player_bullet_query,
    r::ecs::Query<r::ecs::With<EnemyBullet>> enemy_bullet_query,
    r::ecs::Query<r::ecs::Mut<r::Transform3d>, r::ecs::With<Player>> player_query)
{
    for (auto it = enemy_query.begin(); it != enemy_query.end(); ++it) {
        commands.despawn(it.entity());
    }
    for (auto it = player_bullet_query.begin(); it != player_bullet_query.end(); ++it) {
        commands.despawn(it.entity());
    }
    for (auto it = enemy_bullet_query.begin(); it != enemy_bullet_query.end(); ++it) {
        commands.despawn(it.entity());
    }

    for (auto [transform, _] : player_query) {
        transform.ptr->position = {-5.0f, 0.0f, 0.0f};
    }
    spawn_timer.ptr->time_left = ENEMY_SPAWN_INTERVAL;
}

/* ================================================================================= */
/* Main */
/* ================================================================================= */

int main()
{
    /* Seed random for enemy spawn positions */
    srand(static_cast<unsigned int>(time(nullptr)));

    r::Application{}
        /* Setup plugins */
        .add_plugins(r::DefaultPlugins{}.set(
            r::WindowPlugin{r::WindowPluginConfig{
                .size = {1280, 720},
                .title = "R-Type",
                .cursor = r::WindowCursorState::Visible,
            }}
        ))

        /* Initialize state system */
        .init_state(GameState::MainMenu)

        /* Global resources */
        .insert_resource(EnemySpawnTimer{})
        .insert_resource(BossSpawnTimer{})

        /* Global startup */
        .add_systems<setup_ui_theme>(r::Schedule::STARTUP)

        /* Main Menu State */
        .add_systems<build_main_menu>(r::OnEnter{GameState::MainMenu})
        .add_systems<menu_button_handler>(r::Schedule::UPDATE)
            .run_if<r::run_conditions::in_state<GameState::MainMenu>>()
            .after<r::ui::pointer_system>()
            .before<r::ui::clear_click_state_system>()
        .add_systems<cleanup_menu>(r::OnExit{GameState::MainMenu})

        /* InGame State - Setup */
        .add_systems<startup_system, cleanup_system>(r::OnEnter{GameState::EnemiesBattle})

        /* InGame State - Update Systems */
        .add_systems<
            player_input_system,
            movement_system,
            screen_bounds_system,
            collision_system,
            player_collision_system,
            player_bullet_collision_system,
            despawn_offscreen_system,
            setup_boss_fight_system
        >(r::Schedule::UPDATE)
        .add_systems<enemy_spawner_system>(r::Schedule::UPDATE).run_if<r::run_conditions::in_state<GameState::EnemiesBattle>>()
        .add_systems<boss_spawn_system>(r::OnEnter(GameState::BossBattle))
        .add_systems<boss_ai_system>(r::Schedule::UPDATE).run_if<r::run_conditions::in_state<GameState::BossBattle>>()

        /* GameOver State */
        .add_systems<game_over_message_system>(r::OnEnter{GameState::GameOver})
        .add_systems<game_over_system>(r::Schedule::UPDATE)
            .run_if<r::run_conditions::in_state<GameState::GameOver>>()

        .run();

    return 0;
}

/* clang-format on */
