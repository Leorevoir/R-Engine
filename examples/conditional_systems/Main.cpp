#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Core/RunConditions.hpp>
#include <R-Engine/Core/States.hpp>
#include <R-Engine/ECS/Event.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>
#include <iostream>
#include <string>

/* ================================================================================= */
/* Example Objective */
/* */
/* Demonstrate all new `run_if` conditions for controlling system execution: */
/*  - in_state<T>:        Runs a system only when the state machine is in a specific state. */
/*  - state_changed<T>:   Runs a system for one frame when a state transition occurs. */
/*  - resource_exists<T>: Runs a system only when a specific resource is present. */
/*  - on_event<T>:        Runs a system only when a specific event has been fired. */
/* ================================================================================= */

/* The possible states of our application. */
enum class AppState {
    MainMenu,
    Playing,
};

/* A resource that will only exist during the `Playing` state. */
struct GameScore {
        int value = 0;
};

/* An event that is sent when the player performs an action. */
struct PlayerActionEvent {
        std::string description;
};

/* ================================================================================= */
/* Systems */
/* ================================================================================= */

/**
 * @brief Logic for the main menu. Only runs when in the `MainMenu` state.
 */
void menu_system(r::ecs::ResMut<r::NextState<AppState>> next_state)
{
    DrawText("Main Menu", 300, 250, 40, BLACK);
    DrawText("Press ENTER to Play", 280, 300, 20, DARKGRAY);

    if (IsKeyPressed(KEY_ENTER)) {
        next_state.ptr->set(AppState::Playing);
    }
}

/**
 * @brief Core game logic. Only runs when in the `Playing` state.
 * Also sends a `PlayerActionEvent` when the spacebar is pressed.
 */
void game_logic_system(r::ecs::ResMut<r::NextState<AppState>> next_state, r::ecs::EventWriter<PlayerActionEvent> event_writer)
{
    DrawText("Playing!", 320, 250, 40, BLACK);
    DrawText("Press SPACE to trigger an event.", 240, 340, 20, DARKGRAY);
    DrawText("Press M to return to Menu", 250, 300, 20, DARKGRAY);
    DrawFPS(10, 10);

    if (IsKeyPressed(KEY_M)) {
        next_state.ptr->set(AppState::MainMenu);
    }
    if (IsKeyPressed(KEY_SPACE)) {
        event_writer.send({"Player Jumped!"});
    }
}

/**
 * @brief This system only runs when the `GameScore` resource exists.
 * It draws the score to the screen.
 */
void display_score_system(r::ecs::Res<GameScore> score)
{
    DrawText(TextFormat("Score: %d", score.ptr->value), 680, 20, 20, SKYBLUE);
}

/**
 * @brief This system only runs for a single frame when the AppState changes.
 */
void on_state_change_system()
{
    std::cout << "--- state_changed<AppState>: A state transition just occurred! ---" << std::endl;
}

/**
 * @brief This system only runs when a `PlayerActionEvent` has been sent.
 */
void event_listener_system(r::ecs::EventReader<PlayerActionEvent> reader)
{
    for (const auto &event : reader) {
        std::cout << "--- on_event<PlayerActionEvent>: Heard an event: '" << event.description << "' ---" << std::endl;
    }
}

/**
 * @brief System that runs on entering the `MainMenu`. It cleans up game-specific resources.
 */
void cleanup_game_system(r::ecs::Commands commands)
{
    std::cout << "--- OnEnter(MainMenu): Cleaning up game resources. ---" << std::endl;
    commands.remove_resource<GameScore>();
}

/**
 * @brief System that runs on entering the `Playing` state. It sets up game-specific resources.
 */
void setup_game_system(r::ecs::Commands commands)
{
    std::cout << "--- OnEnter(Playing): Setting up game resources (like the score). ---" << std::endl;
    commands.insert_resource(GameScore{0});
}

int main()
{
    r::Application{}
        .add_plugins(r::DefaultPlugins{}.set(r::WindowPlugin{r::WindowPluginConfig{
            .size = {800, 600},
            .title = "Run Conditions Showcase",
        }}))

        /* Initialize the state machine resource, starting in the MainMenu. */
        .init_state(AppState::MainMenu)

        /* Register the event type so the engine knows about it. */
        .add_events<PlayerActionEvent>()

        /* Add systems that run on state transitions for one-time setup/cleanup. */
        .add_systems<cleanup_game_system>(r::OnEnter(AppState::MainMenu))
        .add_systems<setup_game_system>(r::OnEnter(AppState::Playing))

        /* ---- Showcase all run conditions ---- */

        /* 1. `in_state`: Controls the core logic flow. */
        .add_systems<menu_system>(r::Schedule::UPDATE)
        .run_if<r::run_conditions::in_state<AppState::MainMenu>>()

        .add_systems<game_logic_system>(r::Schedule::UPDATE)
        .run_if<r::run_conditions::in_state<AppState::Playing>>()

        /* 2. `state_changed`: Runs for one frame upon any state transition. */
        .add_systems<on_state_change_system>(r::Schedule::UPDATE)
        .run_if<r::run_conditions::state_changed<AppState>>()

        /* 3. `resource_exists`: The score is only displayed if the GameScore resource exists. */
        .add_systems<display_score_system>(r::Schedule::UPDATE)
        .run_if<r::run_conditions::resource_exists<GameScore>>()

        /* 4. `on_event`: Listens for player actions, but only runs when an event is sent. */
        .add_systems<event_listener_system>(r::Schedule::UPDATE)
        .run_if<r::run_conditions::on_event<PlayerActionEvent>>()

        /* Start the application. */
        .run();

    return 0;
}
