#include "game_state.hpp"
#include "../state.hpp"
#include <R-Engine/Application.hpp>

void GameStatePlugin::build(r::Application &app)
{
    app.init_state(GameState::MainMenu);
}
