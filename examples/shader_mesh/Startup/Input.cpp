#include "Startup.hpp"

void r::startup_system_create_inputs(ecs::ResMut<InputMap> input_map) noexcept
{
    auto *map = input_map.ptr;

    map->bindAction("MoveForward", r::KEYBOARD, KEY_W);
    map->bindAction("MoveBackward", r::KEYBOARD, KEY_S);
    map->bindAction("MoveLeft", r::KEYBOARD, KEY_A);
    map->bindAction("MoveRight", r::KEYBOARD, KEY_D);
    map->bindAction("MoveUp", r::KEYBOARD, KEY_SPACE);
    map->bindAction("MoveDown", r::KEYBOARD, KEY_LEFT_CONTROL);
}
