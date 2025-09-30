#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Core/FrameTime.hpp>
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/Maths/Vec.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>
#include <iostream>

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
void spawn_on_click_system(r::ecs::Commands &commands, r::ecs::Res<r::UserInput> userInput,
    r::ecs::Res<r::InputMap> input_map)
{
    if (input_map.ptr->isActionPressed("LeftClick", *userInput.ptr)) {
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
 * @brief (UPDATE) Setup all inputs in the input_map
 * @details This system adds inputs actions to a specific key
 */
static void setup_input_system(r::ecs::Res<r::InputMap> input_map) {
    auto* mutable_map = const_cast<r::InputMap*>(input_map.ptr);

    std::cout << "Binding actions to keys..." << std::endl;
    mutable_map->bindAction("LeftClick", r::MOUSE, MOUSE_LEFT_BUTTON);
    mutable_map->bindAction("MoveForward", r::KEYBOARD, KEY_W);
    mutable_map->bindAction("MoveForward", r::KEYBOARD, KEY_UP);
    mutable_map->bindAction("MoveBackward", r::KEYBOARD ,KEY_S);
    mutable_map->bindAction("MoveLeft", r::KEYBOARD, KEY_A);
    mutable_map->bindAction("MoveRight", r::KEYBOARD, KEY_D);
    mutable_map->bindAction("HideCursor", r::KEYBOARD, KEY_H);
    mutable_map->bindAction("ShowCursor", r::KEYBOARD, KEY_V);
}


/**
 * @brief (UPDATE) Moves the player entity and modifiy the cursor based on keyboard input.
 * @details This system queries for the single entity that has both a `Velocity`
 * component and the `Controllable` marker component.
 */
static void player_control_system(
    r::ecs::Res<r::UserInput> userInput, r::ecs::Res<r::InputMap> input_map,
    r::ecs::Res<r::Cursor> cursor, r::ecs::Query<r::ecs::Mut<Velocity>, r::ecs::With<Controllable>> query)
{
    const float speed = 500.0f;
    for (auto [velocity, _] : query) {
        r::Vec2f direction = {0.0f, 0.0f};
        if (input_map.ptr->isActionPressed("MoveForward", *userInput.ptr))
            direction.y -= 1.0f;
        if (input_map.ptr->isActionPressed("MoveBackward", *userInput.ptr))
            direction.y += 1.0f;
        if (input_map.ptr->isActionPressed("MoveLeft", *userInput.ptr))
            direction.x -= 1.0f;
        if (input_map.ptr->isActionPressed("MoveRight", *userInput.ptr))
            direction.x += 1.0f;

        if (direction.length() > 0.0f) {
            velocity.ptr->value = direction.normalize() * speed;
        } else {
            velocity.ptr->value = {0.0f, 0.0f};
        }
    }

    auto* mutable_cursor = const_cast<r::Cursor*>(cursor.ptr);
    if (input_map.ptr->isActionPressed("HideCursor", *userInput.ptr)) {
        mutable_cursor->state = r::WindowCursorState::Hidden;
    }

    //Show the cursor
    if (input_map.ptr->isActionPressed("ShowCursor", *userInput.ptr)) {
        mutable_cursor->state = r::WindowCursorState::Visible;
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

// clang-format off

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
                        .size = {800, 600},
                        .title = "Bevy Style Configuration!",
                        .cursor = r::WindowCursorState::Visible,
                    }
                })
        )
        /* Example of adding plugins separately */
        /* .add_plugins(r::InputPlugin{}, r::RenderPlugin{}, r::WindowPlugin{}) */

        /* Insert global resources. These can be accessed by systems. */
        .insert_resource(Gravity{})

        /* Add systems to the application schedule. */
        /* STARTUP systems run once at the beginning. */
        .add_systems(r::Schedule::STARTUP, spawn_entities_system, setup_input_system)

        /* UPDATE systems run once every frame for game logic and physics. */
        /* The order matters here: input -> physics -> movement. */
        .add_systems(r::Schedule::UPDATE,
            spawn_on_click_system,
            player_control_system,
            apply_gravity_system,
            move_system,
            bounce_system
        )

        /* RENDER systems run after UPDATE systems for drawing. */
        /* The RenderPlugin already adds systems to begin and end the drawing context. */
        .add_systems(r::Schedule::RENDER, render_system)

        /* Start the main application loop. */
        .run();

    return 0;
}
/* clang-format on */
