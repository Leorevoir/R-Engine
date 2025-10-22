#include "../Game.hpp"
#include "Startup.hpp"

// clang-format off

void r::startup_system_create_player(ecs::Commands &commands) noexcept
{
    commands.spawn(
        Controllable{},
        Player{},
        Transform3d{
            .position = {0, 1, 0}
        },
        Velocity{
            {.0f, .0f, .0f}
        }
    );
}
