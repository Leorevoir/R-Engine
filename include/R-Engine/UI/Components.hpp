#pragma once

#include <R-Engine/R-EngineExport.hpp>
#include <R-Engine/Types.hpp>
#include <R-Engine/ECS/Entity.hpp>

#include <vector>

namespace r {

// Marker component for UI entities
struct R_ENGINE_API UiNode {
};

// Hierarchy
struct R_ENGINE_API Parent {
    ecs::Entity id = 0; // 0 -> no parent
};

struct R_ENGINE_API Children {
    std::vector<ecs::Entity> ids;
};

// Visibility
enum class Visibility : u8 {
    Visible,
    Hidden,
    Collapsed,
};

// Basic style (Phase 2): fixed size + colors + margins
struct R_ENGINE_API Style {
    f32 width = 0.f;         // px (0 -> auto)
    f32 height = 0.f;        // px (0 -> auto)
    Color background{200, 200, 200, 200};
    i32 z_index = 0;
    f32 margin = 0.f;        // uniform margin in px
    f32 padding = 0.f;       // uniform padding in px
};

// Computed (absolute) layout in screen-space
struct R_ENGINE_API ComputedLayout {
    f32 x = 0.f;
    f32 y = 0.f;
    f32 w = 0.f;
    f32 h = 0.f;
    i32 z = 0;
};

} // namespace r

