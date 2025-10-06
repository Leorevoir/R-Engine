#pragma once

#include <R-Engine/R-EngineExport.hpp>
#include <R-Engine/Maths/Vec.hpp>
#include <R-Engine/ECS/Entity.hpp>

namespace r {

/**
 * @brief Per-frame UI input state (mouse only for now).
 */
struct R_ENGINE_API UiInputState {
    Vec2f mouse_position{0.f, 0.f};
    bool mouse_left_down = false;
    bool mouse_left_pressed = false;
    bool mouse_left_released = false;

    ecs::Entity hovered = 0;
    ecs::Entity prev_hovered = 0;
    ecs::Entity active = 0;
    ecs::Entity focused = 0;
};

} /* namespace r */
