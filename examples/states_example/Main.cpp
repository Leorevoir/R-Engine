#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Core/States.hpp>
#include <R-Engine/ECS/Event.hpp>
#include <R-Engine/ECS/RunConditions.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>

#include <iostream>
#include <string>

enum class AppState { MainMenu, Playing, Paused };

struct GameScore {
        int value = 0;
};

struct PlayerActionEvent {
        std::string description;
};

/**
 * systems
 */

static void menu_system(r::ecs::ResMut<r::NextState<AppState>> next_state)
{
    DrawText("Main Menu (Press ENTER)", 250, 250, 30, BLACK);
}

static void game_logic_system(r::ecs::ResMut<r::NextState<AppState>> next_state, r::ecs::EventWriter<PlayerActionEvent> event_writer)
{
    DrawText("Playing!", 320, 250, 40, BLACK);
    DrawText("Press P to Pause", 300, 300, 20, DARKGRAY);
    DrawText("Press SPACE to trigger an event", 240, 340, 20, DARKGRAY);

    if (IsKeyPressed(KEY_P)) {
        next_state.ptr->set(AppState::Paused);
    }
    if (IsKeyPressed(KEY_SPACE)) {
        event_writer.send({"Player Jumped!"});
    }
}

static void paused_overlay_system(r::ecs::ResMut<r::NextState<AppState>> next_state)
{
    DrawRectangle(0, 0, 800, 600, {0, 0, 0, 100});
    DrawText("PAUSED", 320, 250, 40, RAYWHITE);
    DrawText("Press P to Resume", 300, 300, 20, LIGHTGRAY);
    if (IsKeyPressed(KEY_P)) {
        next_state.ptr->set(AppState::Playing);
    }
}

static void display_score_system(r::ecs::Res<GameScore> score)
{
    DrawText(TextFormat("Score: %d", score.ptr->value), 680, 20, 20, SKYBLUE);
}

static void playing_or_paused_system()
{
    DrawText("State: Playing or Paused", 10, 40, 20, GREEN);
}

static void playing_and_event_system(r::ecs::EventReader<PlayerActionEvent> reader)
{
    for (const auto &event : reader) {
        std::cout << "--- 'Playing' AND 'on_event' system fired with event: " << event.description << " ---" << std::endl;
    }
}

static void not_in_menu_system()
{
    DrawText("NOT in Main Menu", 10, 70, 20, MAROON);
}

static void state_control_system(r::ecs::Res<r::State<AppState>> state, r::ecs::ResMut<r::NextState<AppState>> next_state)
{
    if (state.ptr->current() == AppState::MainMenu && IsKeyPressed(KEY_ENTER)) {
        next_state.ptr->set(AppState::Playing);
    }
    DrawFPS(10, 10);
}

static void cleanup_game_system(r::ecs::Commands commands)
{
    std::cout << "--- OnEnter(MainMenu): Cleaning up game resources. ---" << std::endl;
    commands.remove_resource<GameScore>();
}

static void setup_game_system(r::ecs::Commands commands)
{
    std::cout << "--- OnEnter(Playing): Setting up game resources (like the score). ---" << std::endl;
    commands.insert_resource(GameScore{0});
}

int main(void)
{
    r::Application{}
        .add_plugins(r::DefaultPlugins{}.set(r::WindowPlugin{r::WindowPluginConfig{
            .size = {800, 600},
            .title = "Combined Run Conditions Showcase",
        }}))
        .init_state(AppState::MainMenu)
        .add_events<PlayerActionEvent>()

        .add_systems<cleanup_game_system>(r::OnEnter(AppState::MainMenu))
        .add_systems<setup_game_system>(r::OnEnter(AppState::Playing))
        .add_systems<state_control_system>(r::Schedule::UPDATE)

        .add_systems<menu_system>(r::Schedule::UPDATE)
        .run_if<r::run_conditions::in_state<AppState::MainMenu>>()
        .add_systems<game_logic_system>(r::Schedule::UPDATE)
        .run_if<r::run_conditions::in_state<AppState::Playing>>()
        .add_systems<paused_overlay_system>(r::Schedule::UPDATE)
        .run_if<r::run_conditions::in_state<AppState::Paused>>()

        .add_systems<playing_or_paused_system>(r::Schedule::UPDATE)
        .run_if<r::run_conditions::in_state<AppState::Playing>>()
        .run_or<r::run_conditions::in_state<AppState::Paused>>()

        .add_systems<playing_and_event_system>(r::Schedule::UPDATE)
        .run_if<r::run_conditions::in_state<AppState::Playing>>()
        .run_and<r::run_conditions::on_event<PlayerActionEvent>>()

        .add_systems<not_in_menu_system>(r::Schedule::UPDATE)
        .run_unless<r::run_conditions::in_state<AppState::MainMenu>>()

        .add_systems<display_score_system>(r::Schedule::UPDATE)
        .run_if<r::run_conditions::resource_exists<GameScore>>()

        .run();

    return 0;
}
