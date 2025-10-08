#pragma once

#include <R-Engine/R-EngineExport.hpp>
#include <R-Engine/Types.hpp>
#include <R-Engine/ECS/Entity.hpp>

#include <vector>

namespace r {

/* Marker component for UI entities */
struct R_ENGINE_API UiNode {
};

/* UI handle + logical parent: ECS-agnostic IDs used by UiPlugin */
struct R_ENGINE_API UiId { u32 value = 0; };
struct R_ENGINE_API UiParent { u32 handle = 0; };
struct R_ENGINE_API UiIdGen { u32 next = 1; };

/* Focusable marker (optional). By default UiButton is treated as focusable. */
struct R_ENGINE_API UiFocusable {
};

/* Hierarchy */
struct R_ENGINE_API Parent {
    ecs::Entity id = 0; /* 0 -> no parent */
};

struct R_ENGINE_API Children {
    std::vector<ecs::Entity> ids;
};

/* Visibility */
enum class Visibility : u8 {
    Visible,
    Hidden,
    Collapsed,
};

/* Layout enums */
enum class LayoutDirection : u8 { Row, Column };
enum class JustifyContent : u8 { Start, Center, End, SpaceBetween };
enum class AlignItems : u8 { Start, Center, End, Stretch };
enum class AlignSelf : u8 { Auto, Start, Center, End, Stretch };
enum class PositionType : u8 { Relative, Absolute };

/* Basic style (Phase 3): fixed size + colors + margins + layout */
struct R_ENGINE_API Style {
    f32 width = 0.f;         /* px (0 -> auto) */
    f32 height = 0.f;        /* px (0 -> auto) */
    f32 width_pct = -1.f;    /* percent [0..100], -1 -> unset */
    f32 height_pct = -1.f;   /* percent [0..100], -1 -> unset */
    f32 min_width = 0.f;     /* 0 -> unset */
    f32 max_width = 0.f;     /* 0 -> unset */
    f32 min_height = 0.f;    /* 0 -> unset */
    f32 max_height = 0.f;    /* 0 -> unset */
    Color background{200, 200, 200, 200};
    i32 z_index = 0;
    i32 order = 0;           /* layout ordering within parent (lower comes first) */
    f32 margin = 0.f;        /* uniform margin in px */
    f32 padding = 0.f;       /* uniform padding in px */
    LayoutDirection direction = LayoutDirection::Column;
    JustifyContent justify = JustifyContent::Start;
    AlignItems align = AlignItems::Start;
    AlignSelf align_self = AlignSelf::Auto;
    PositionType position = PositionType::Relative;
    f32 offset_x = 0.f;      /* for absolute positioning */
    f32 offset_y = 0.f;      /* for absolute positioning */
    f32 gap = 0.f;           /* spacing between children; 0 -> theme spacing */
    bool clip_children = false;      /* if true, clip descendants to this node's content rect */
    bool overflow_clip = false;      /* synonym; if true, also enables clipping */
    f32 border_thickness = 0.f;      /* px, 0 = no border */
    Color border_color{0, 0, 0, 255};
};

/* Computed (absolute) layout in screen-space */
struct R_ENGINE_API ComputedLayout {
    f32 x = 0.f;
    f32 y = 0.f;
    f32 w = 0.f;
    f32 h = 0.f;
    i32 z = 0;
};

/* Optional scroll component for containers. */
struct R_ENGINE_API UiScroll {
    f32 x = 0.f;
    f32 y = 0.f;
};

} /* namespace r */
