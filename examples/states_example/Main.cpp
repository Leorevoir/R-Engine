#include "R-Engine/ECS/Query.hpp"
#include "raylib.h"
#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>
#include <R-Engine/Core/States.hpp>
#include <iostream>

enum class AppState { MainMenu, Playing };

void setup_menu() { std::cout << "--- OnEnter: MainMenu ---" << std::endl; }
void cleanup_menu() { std::cout << "--- OnExit: MainMenu ---" << std::endl; }

void menu_logic(r::ecs::ResMut<r::NextState<AppState>> next_state) {
    if (IsKeyPressed(KEY_ENTER)) {
        std::cout << "INPUT: ENTER pressed, transition to Playing..." << std::endl;
        next_state.ptr->set(AppState::Playing);
    }
}

void game_logic(r::ecs::ResMut<r::NextState<AppState>> next_state) {
    if (IsKeyPressed(KEY_UP)) {
        std::cout << "INPUT: UP pressed, back to MainMenu..." << std::endl;
        next_state.ptr->set(AppState::MainMenu);
    }
}

void spawn_player_on_start() { 
    std::cout << "--- OnTransition: MainMenu -> Playing (Spawning player) ---" << std::endl; 
}

void get_state(r::ecs::Res<r::State<AppState>> state)
{
    AppState current_state = state.ptr->current();

    switch (current_state) {
        case AppState::MainMenu:
            std::cout << "DEBUG: Current state is MainMenu" << std::endl;
            break;
        case AppState::Playing:
            std::cout << "DEBUG: Current state is Playing" << std::endl;
            break;
    }

    if (state.ptr->previous().has_value()) {
        AppState previous_state = state.ptr->previous().value();
    }
}

/**
 * @brief Temporary system function to update game state
 */
void update_logic(r::ecs::Res<r::State<AppState>> state, r::ecs::ResMut<r::NextState<AppState>> next_state)
{
    switch (state.ptr->current())
    {
        case AppState::MainMenu:
            if (IsKeyPressed(KEY_ENTER)) {
                std::cout << "INPUT: ENTER pressed, transition to Playing..." << std::endl;
                next_state.ptr->set(AppState::Playing);
            }
            break;

        case AppState::Playing:
            if (IsKeyPressed(KEY_UP)) {
                std::cout << "INPUT: UP pressed, back to MainMenu..." << std::endl;
                next_state.ptr->set(AppState::MainMenu);
            }
            break;
    }
}

int main() {
    r::Application{}
        .add_plugins(
            r::DefaultPlugins{}
                .set(r::WindowPlugin{
                    r::WindowPluginConfig{
                        .size = {800, 600},
                        .title = "Game states example",
                        .cursor = r::WindowCursorState::Visible,
                    }
                })
        )
        .init_state(AppState::MainMenu)

        .add_systems<update_logic>(r::Schedule::UPDATE)
        .add_systems<setup_menu>(r::OnEnter(AppState::MainMenu))
        .add_systems<cleanup_menu>(r::OnExit(AppState::MainMenu))
        .add_systems<spawn_player_on_start>(r::OnTransition(AppState::MainMenu, AppState::Playing))
        .add_systems<get_state>(r::Schedule::UPDATE)

        .run();

    return 0;
}