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

    u32 hovered = 0;
    u32 prev_hovered = 0;
    u32 active = 0;
    u32 focused = 0;
};

} /* namespace r */
