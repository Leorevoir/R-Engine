#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Core/States.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>
#include <iostream>

/* ================================================================================= */
/* Example Objective */
/* */
/* Demonstrate the `run_if` feature for conditional system execution. */
/* We will create a simple state machine with two states: MainMenu and Playing. */
/* */
/* - A `menu_system` will only run when the application is in the MainMenu state. */
/* - A `game_logic_system` will only run when the application is in the Playing state. */
/* */
/* This avoids manual `if (state == ...)` checks inside systems, making them cleaner */
/* and more focused on their specific tasks. */
/* ================================================================================= */

/* The possible states of our application. */
enum class AppState {
    MainMenu,
    Playing,
};

/* ================================================================================= */
/* Predicate Functions */
/* */
/* These are simple functions that return a boolean and can accept any valid system */
/* parameter. The scheduler uses them to decide whether to run a system. */
/* ================================================================================= */

/**
 * @brief A predicate that returns `true` if the current `AppState` is `MainMenu`.
 */
bool is_in_main_menu(r::ecs::Res<r::State<AppState>> state)
{
    return state.ptr->current() == AppState::MainMenu;
}

/**
 * @brief A predicate that returns `true` if the current `AppState` is `Playing`.
 */
bool is_in_game(r::ecs::Res<r::State<AppState>> state)
{
    return state.ptr->current() == AppState::Playing;
}

/* ================================================================================= */
/* Systems */
/* */
/* Note how each system is only concerned with its own logic. It doesn't need to */
/* know about other states. The `run_if` condition handles that separation. */
/* ================================================================================= */

/**
 * @brief Logic and rendering for the main menu.
 * @details This system is only active when `is_in_main_menu` returns true.
 * It listens for the Enter key to transition to the `Playing` state.
 */
void menu_system(r::ecs::ResMut<r::NextState<AppState>> next_state)
{
    DrawText("Main Menu", 300, 250, 40, BLACK);
    DrawText("Press ENTER to Play", 280, 300, 20, DARKGRAY);

    if (IsKeyPressed(KEY_ENTER)) {
        std::cout << "Transitioning to state: Playing" << std::endl;
        next_state.ptr->set(AppState::Playing);
    }
}

/**
 * @brief Core game logic.
 * @details This system is only active when `is_in_game` returns true.
 * It listens for the Escape key to transition back to the `MainMenu` state.
 */
void game_logic_system(r::ecs::ResMut<r::NextState<AppState>> next_state)
{
    DrawText("Playing!", 320, 250, 40, BLACK);
    DrawText("Press ESC to return to Menu", 250, 300, 20, DARKGRAY);
    DrawFPS(10, 10);

    if (IsKeyPressed(KEY_ESCAPE)) {
        std::cout << "Transitioning to state: MainMenu" << std::endl;
        next_state.ptr->set(AppState::MainMenu);
    }
}

/**
 * @brief A system that runs once when entering the MainMenu state.
 * @details This shows how `OnEnter` events complement `run_if`. `OnEnter` is for
 * setup, while `run_if` is for continuous logic within the state.
 */
void setup_menu_system()
{
    std::cout << "--- OnEnter(MainMenu): Menu setup logic runs here. ---" << std::endl;
}

/**
 * @brief A system that runs once when entering the Playing state.
 */
void setup_game_system()
{
    std::cout << "--- OnEnter(Playing): Game setup logic (e.g., spawn player) runs here. ---" << std::endl;
}

int main()
{
    r::Application{}
        .add_plugins(r::DefaultPlugins{}.set(r::WindowPlugin{r::WindowPluginConfig{
            .size = {800, 600},
            .title = "Conditional Systems (`run_if`) Example",
        }}))

        // Initialize the state machine resource, starting in the MainMenu.
        .init_state(AppState::MainMenu)

        // Add systems that run on state transitions for one-time setup/cleanup.
        .add_systems<setup_menu_system>(r::OnEnter(AppState::MainMenu))
        .add_systems<setup_game_system>(r::OnEnter(AppState::Playing))

        /* Add the main logic systems and gate them with `run_if`. */
        .add_systems<menu_system>(r::Schedule::UPDATE)
        .run_if<is_in_main_menu>()

        .add_systems<game_logic_system>(r::Schedule::UPDATE)
        .run_if<is_in_game>()

        /* Start the application. */
        .run();

    return 0;
}
