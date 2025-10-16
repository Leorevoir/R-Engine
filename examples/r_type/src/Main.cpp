#include "plugins/combat.hpp"
#include "plugins/game_state.hpp"
#include "plugins/gameplay.hpp"
#include "plugins/menu.hpp"
#include "plugins/player.hpp"
#include "state.hpp"

#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/Plugins/RenderPlugin.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>

#include <cstdlib>
#include <ctime>

/**
 * @brief (STARTUP) Sets up the game world.
 * @details This system runs once when entering the game. It configures the camera
 * and binds input actions.
 */
static void setup_core_game_system(r::ecs::ResMut<r::Camera3d> camera, r::ecs::ResMut<r::InputMap> input_map)
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
}

int main()
{
    /* Seed random for enemy spawn positions */
    srand(static_cast<unsigned int>(time(nullptr)));

    r::Application{}
        .add_plugins(r::DefaultPlugins{}.set(r::WindowPlugin{r::WindowPluginConfig{
            .size = {1280, 720},
            .title = "R-Type",
            .cursor = r::WindowCursorState::Visible,
        }}))

        /* Add all our custom game plugins */
        .add_plugins(GameStatePlugin{})
        .add_plugins(MenuPlugin{})
        .add_plugins(PlayerPlugin{})
        .add_plugins(GameplayPlugin{})
        .add_plugins(CombatPlugin{})

        /* Add the remaining core setup */
        .add_systems<setup_core_game_system>(r::OnEnter{GameState::EnemiesBattle})

        .run();

    return 0;
}
