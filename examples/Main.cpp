#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Core/FrameTime.hpp>
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/Maths/Vec.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>
#include <R-Engine/Plugins/UIPlugin.hpp>

#include <cstdlib>
#include <ctime>

/* clang-format off */

/* ================================================================================= */
/* Components */
/* */
/* Components are simple data structures that are attached to entities. */
/* They define the properties and state of an entity. */
/* ================================================================================= */

/* Represents a 2D position in space. */
struct Position {
    r::Vec2f value;
};

/* Represents a 2D velocity. */
struct Velocity {
    r::Vec2f value;
};

/* Represents a drawable circle with a radius and color. */
struct Circle {
    float radius;
    Color color;
};

/* A "marker component" for the entity that can be controlled by the player. */
/* It has no data, its presence alone is used to identify the player entity in queries. */
struct Controllable {};

/* ================================================================================= */
/* Resources */
/* */
/* Resources are global, singleton data structures accessible by any system. */
/* They are useful for storing application-wide state. */
/* ================================================================================= */

/* A global resource for gravity. */
struct Gravity {
    r::Vec2f value = {0.0f, 980.0f};
};

/* ================================================================================= */
/* Systems */
/* */
/* Systems are functions that implement the logic of your application. */
/* They query for entities with specific components and operate on them. */
/* ================================================================================= */

/**
 * @brief (STARTUP) Spawns the initial entities in the world.
 * @details This system runs once when the application starts. It uses `ecs::Commands`
 * to create new entities and add components to them. It also uses `ecs::Res` to
 * get the window configuration to place entities correctly.
 */
void spawn_entities_system(r::ecs::Commands &commands, r::ecs::Res<r::WindowPluginConfig> win_config)
{
    const float screen_width = static_cast<float>(win_config.ptr->size.width);
    const float screen_height = static_cast<float>(win_config.ptr->size.height);

    /* Spawn the player with all its components in one go. */
    commands.spawn(
        Controllable{},
        Circle{
            .radius = 20.0f,
            .color  = BLUE
        },
        Position{
            .value = {screen_width / 2.0f, screen_height / 2.0f}
        },
        Velocity{
            .value = {0.0f, 0.0f}
        }
    );

    /* Spawn 10 random balls. */
    for (int i = 0; i < 10; ++i) {
        float radius = static_cast<float>((rand() % 20) + 10);

        commands.spawn(
            Position{
                .value = {
                    static_cast<float>(rand() % (int)(screen_width - radius * 2) + radius),
                    static_cast<float>(rand() % (int)(screen_height / 2) + radius)
                }
            },
            Velocity{
                .value = {
                    static_cast<float>((rand() % 500) - 250),
                    static_cast<float>((rand() % 500) - 250)
                }
            },
            Circle{
                .radius = radius,
                .color  = {(unsigned char)(rand() % 256), (unsigned char)(rand() % 256), (unsigned char)(rand() % 256), 255}
            }
        );
    }
}

/**
 * @brief (UPDATE) Spawns a new ball on mouse click.
 */
void spawn_on_click_system(r::ecs::Commands &commands)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse_pos = GetMousePosition();
        float radius = static_cast<float>((rand() % 15) + 5);

        commands.spawn(
            Position{
                .value = {mouse_pos.x, mouse_pos.y}
            },
            Velocity{
                .value = {static_cast<float>((rand() % 600) - 300), static_cast<float>((rand() % 600) - 300)}
            },
            Circle{
                .radius = radius,
                .color = {(unsigned char)(rand() % 156 + 100), (unsigned char)(rand() % 156 + 100), (unsigned char)(rand() % 156 + 100), 255}
            }
        );
    }
}

/**
 * @brief (UPDATE) Moves the player entity based on keyboard input.
 * @details This system queries for the single entity that has both a `Velocity`
 * component and the `Controllable` marker component.
 */
void player_control_system(r::ecs::Query<r::ecs::Mut<Velocity>, r::ecs::With<Controllable>> query)
{
    const float speed = 500.0f;
    for (auto [velocity, _] : query) {
        r::Vec2f direction = {0.0f, 0.0f};
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
            direction.y -= 1.0f;
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
            direction.y += 1.0f;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
            direction.x -= 1.0f;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
            direction.x += 1.0f;

        if (direction.length() > 0.0f) {
            velocity.ptr->value = direction.normalize() * speed;
        } else {
            velocity.ptr->value = {0.0f, 0.0f};
        }
    }
}

/**
 * @brief (UPDATE) Applies gravity to entities that are not controllable.
 * @details This system queries for entities with `Velocity` but `Without` the
 * `Controllable` component, so it only affects the balls.
 */
void apply_gravity_system(r::ecs::Query<r::ecs::Mut<Velocity>, r::ecs::Without<Controllable>> query, r::ecs::Res<Gravity> gravity,
    r::ecs::Res<r::core::FrameTime> time)
{
    for (auto [velocity, _] : query) {
        velocity.ptr->value += gravity.ptr->value * time.ptr->delta_time;
    }
}

/**
 * @brief (UPDATE) Updates entity positions based on their velocity.
 * @details A classic physics integration step. It queries for all entities that have
 * both `Position` and `Velocity`.
 */
void move_system(r::ecs::Query<r::ecs::Mut<Position>, r::ecs::Ref<Velocity>> query, r::ecs::Res<r::core::FrameTime> time)
{
    for (auto [position, velocity] : query) {
        position.ptr->value += velocity.ptr->value * time.ptr->delta_time;
    }
}

/**
 * @brief (UPDATE) Makes entities bounce off the screen edges.
 */
void bounce_system(r::ecs::Query<r::ecs::Mut<Position>, r::ecs::Mut<Velocity>, r::ecs::Ref<Circle>> query,
    r::ecs::Res<r::WindowPluginConfig> win_config)
{
    const float screen_width = static_cast<float>(win_config.ptr->size.width);
    const float screen_height = static_cast<float>(win_config.ptr->size.height);
    const float damping = 0.8f;

    for (auto [position, velocity, circle] : query) {
        if (position.ptr->value.x - circle.ptr->radius < 0) {
            position.ptr->value.x = circle.ptr->radius;
            velocity.ptr->value.x = -velocity.ptr->value.x * damping;
        }
        if (position.ptr->value.x + circle.ptr->radius > screen_width) {
            position.ptr->value.x = screen_width - circle.ptr->radius;
            velocity.ptr->value.x = -velocity.ptr->value.x * damping;
        }
        if (position.ptr->value.y - circle.ptr->radius < 0) {
            position.ptr->value.y = circle.ptr->radius;
            velocity.ptr->value.y = -velocity.ptr->value.y * damping;
        }
        if (position.ptr->value.y + circle.ptr->radius > screen_height) {
            position.ptr->value.y = screen_height - circle.ptr->radius;
            velocity.ptr->value.y = -velocity.ptr->value.y * damping;
        }
    }
}

/**
 * @brief (RENDER) Draws all entities with Position and Circle components.
 * @details This system runs in the RENDER schedule, after all the UPDATE logic.
 * The RenderPlugin ensures the drawing context is set up before this system runs.
 */
void render_system(r::ecs::Query<r::ecs::Ref<Position>, r::ecs::Ref<Circle>> query)
{
    for (auto [position, circle] : query) {
        DrawCircleV({position.ptr->value.x, position.ptr->value.y}, circle.ptr->radius, circle.ptr->color);
    }

    DrawText("WASD/Arrows to move, Left-Click to spawn balls", 10, 10, 20, DARKGRAY);
    DrawFPS(10, 30);
}

/**
 * @brief (STARTUP) Spawns a very simple main menu UI: title + Play + Quit buttons.
 * Play currently does nothing; Quit uses UiOnClickQuit component to exit.
 */
void spawn_ui_menu_system(r::ecs::Commands &commands, r::ecs::Res<r::WindowPluginConfig> win_config)
{
    const float screen_width = static_cast<float>(win_config.ptr->size.width);
    const float screen_height = static_cast<float>(win_config.ptr->size.height);

    /* Title text centered at top */
    commands.spawn(
        r::UiText{ .value = "R-Type", .size = 64 },
        r::UiTextColor{ 0, 0, 0, 255 },
        r::UiPosition{ { (screen_width - 400.f) * 0.5f, 60.f } }, /* rough centering; actual centering handled by rect if present */
        r::UiZIndex{ 10 }
    );

    /* Common button sizes */
    const r::Vec2f btn_size = { 220.f, 70.f };
    const float center_x = (screen_width - btn_size.x) * 0.5f;
    float start_y = screen_height * 0.5f - 80.f; /* first button y */

    /* Play button via bundle helper */
    {
        r::UiButtonBundle play_bundle{};
        play_bundle.text.value = "Play";
        play_bundle.original_color = {90,140,255,255};
        play_bundle.color = {play_bundle.original_color.r, play_bundle.original_color.g, play_bundle.original_color.b, play_bundle.original_color.a};
        /* Style override example: faster hover darkening & stronger flash */
        play_bundle.style.hover_dark_percent = 0.65f;
        play_bundle.style.flash_percent = 1.3f;
        play_bundle.rect.size = btn_size;
        r::spawn_ui_button(commands, play_bundle, r::UiPosition{{center_x, start_y}});
    }

    /* Options button with a different base color (shows style override) */
    {
        r::UiButtonBundle options_bundle{};
        options_bundle.text.value = "Options";
        options_bundle.original_color = {60, 180, 110, 255};
        options_bundle.color = {options_bundle.original_color.r, options_bundle.original_color.g, options_bundle.original_color.b, options_bundle.original_color.a};
        options_bundle.style.hover_dark_percent = 0.4f; /* lighter darkening */
        options_bundle.style.flash_percent = 0.8f;      /* smaller flash */
        options_bundle.rect.size = btn_size;
        r::spawn_ui_button(commands, options_bundle, r::UiPosition{{center_x, start_y + btn_size.y + 30.f}});
    }

    /* Disabled button example (non-interactive, styled via disabled flag) */
    {
        r::UiButtonBundle disabled_bundle{};
        disabled_bundle.text.value = "Disabled";
        disabled_bundle.original_color = {120, 120, 120, 255};
        disabled_bundle.color = {disabled_bundle.original_color.r, disabled_bundle.original_color.g, disabled_bundle.original_color.b, disabled_bundle.original_color.a};
        disabled_bundle.style.disabled = true;
        disabled_bundle.rect.size = btn_size;
        r::spawn_ui_button(commands, disabled_bundle, r::UiPosition{{center_x, start_y + (btn_size.y + 30.f) * 2.f}});
    }

    /* Quit button via bundle helper + explicit quit action */
    {
        r::UiButtonBundle quit_bundle{};
        quit_bundle.text.value = "Quit";
        quit_bundle.original_color = {200,60,80,255};
        quit_bundle.color = {quit_bundle.original_color.r, quit_bundle.original_color.g, quit_bundle.original_color.b, quit_bundle.original_color.a};
        quit_bundle.border = {90,20,30,255};
        quit_bundle.rect.size = btn_size;
        auto e = r::spawn_ui_button(commands, quit_bundle, r::UiPosition{{center_x, start_y + (btn_size.y + 30.f) * 3.f}});
        /* Add the quit action component */
        commands.entity(e).insert(r::UiOnClickQuit{});
    }
}

/* ========================================================================== */
/* (UPDATE) Example consumer of the UiClickEvents bus                         */
/* ========================================================================== */
void ui_click_events_logger(r::ecs::Res<r::UiEvents> events)
{
    /* Manual consumer example (logger system built-in can also be enabled via UiEventLoggerConfig). */
    for (const auto &ev : events.ptr->current) {
        if (ev.type == r::UiEventType::QuitClick) {
            TraceLog(LOG_INFO, "[UI] Quit button clicked (label=%s)", ev.label.c_str());
        }
    }
}

/* ========================================================================== */
/* (STARTUP) Configure UI logger & tweak global theme                         */
/* ========================================================================== */
void configure_ui_logger_and_theme(r::ecs::Res<r::UiEventLoggerConfig> logger_cfg, r::ecs::Res<r::UiTheme> theme)
{
    auto *cfg = const_cast<r::UiEventLoggerConfig*>(logger_cfg.ptr);
    cfg->enabled = true;
    cfg->log_hover = true;
    cfg->log_press = true;
    cfg->log_release = true;
    cfg->log_click = true;
    cfg->log_quit = true;

    /* Light theme tweaks (faster hover & stronger flash globally) */
    auto *th = const_cast<r::UiTheme*>(theme.ptr);
    th->hover_speed = 18.f;
    th->flash_percent = 1.1f;
}

/* ========================================================================== */
/* (RENDER) UI Stats & Event overlay (demonstrates UiStats + UiEvents)        */
/* ========================================================================== */
void ui_debug_overlay_render(r::ecs::Res<r::UiStats> stats, r::ecs::Res<r::UiEvents> events)
{
    const int x = 10;
    int y = 70; /* below existing debug text */
    char buf[256];
    std::snprintf(buf, sizeof(buf), "UI Frame %llu", (unsigned long long)stats.ptr->frame_index);
    DrawText(buf, x, y, 16, DARKGREEN); y += 18;
    std::snprintf(buf, sizeof(buf), "Events: %u (curr) Rects: %u Texts: %u", stats.ptr->events_emitted, stats.ptr->rects_drawn, stats.ptr->texts_drawn);
    DrawText(buf, x, y, 14, DARKGREEN); y += 16;
    std::snprintf(buf, sizeof(buf), "Interaction Ent: %u", stats.ptr->interaction_entities);
    DrawText(buf, x, y, 14, DARKGREEN); y += 16;
    std::snprintf(buf, sizeof(buf), "Times ms I%.2f S%.2f Rr%.2f Rt%.2f", stats.ptr->interaction_ms, stats.ptr->style_ms, stats.ptr->render_rect_ms, stats.ptr->render_text_ms);
    DrawText(buf, x, y, 14, DARKGREEN); y += 18;
    /* Count event types this frame */
    unsigned hoverEnter=0, hoverLeave=0, pressed=0, released=0, click=0, quitClick=0;
    for (auto &e : events.ptr->current) {
        switch (e.type) {
            case r::UiEventType::HoverEnter: ++hoverEnter; break;
            case r::UiEventType::HoverLeave: ++hoverLeave; break;
            case r::UiEventType::Pressed: ++pressed; break;
            case r::UiEventType::Released: ++released; break;
            case r::UiEventType::Click: ++click; break;
            case r::UiEventType::QuitClick: ++quitClick; break;
            default: break;
        }
    }
    std::snprintf(buf, sizeof(buf), "Ev H+%u H-%u P%u R%u C%u Q%u", hoverEnter, hoverLeave, pressed, released, click, quitClick);
    DrawText(buf, x, y, 14, DARKGREEN);
}

int main()
{
    /* Seed random number generator for varied colors and velocities. */
    srand(static_cast<unsigned int>(time(nullptr)));

    r::Application{}
        /* Add default plugins like windowing and rendering. */
        /* We can configure a plugin by creating an instance and passing it to .set(). */
        .add_plugins(
            r::DefaultPlugins{}
                .set(r::WindowPlugin{
                    r::WindowPluginConfig{
                        .size = {1280, 720},
                        .title = "R-Engine: Bundle Spawn Demo",
                    }
                })
        )

        /* Insert global resources. These can be accessed by systems. */
        .insert_resource(Gravity{})

        /* Add systems to the application schedule. */
        /* STARTUP systems run once at the beginning. */
        .add_systems(r::Schedule::STARTUP, spawn_entities_system)
        .add_systems(r::Schedule::STARTUP, spawn_ui_menu_system)
        .add_systems(r::Schedule::STARTUP, configure_ui_logger_and_theme)

        /* UPDATE systems run once every frame for game logic and physics. */
        /* The order matters here: input -> physics -> movement. */
        .add_systems(r::Schedule::UPDATE,
            spawn_on_click_system,
            player_control_system,
            apply_gravity_system,
            move_system,
            bounce_system,
            ui_click_events_logger /* placed after UI plugin systems so events are populated */
        )

        /* RENDER systems run after UPDATE systems for drawing. */
        /* The RenderPlugin already adds systems to begin and end the drawing context. */
    .add_systems(r::Schedule::RENDER, render_system)
    .add_systems(r::Schedule::RENDER, ui_debug_overlay_render)

        /* Start the main application loop. */
        .run();

    return 0;
}
/* clang-format on */
