#pragma once

#include <R-Engine/Plugins/Plugin.hpp>
#include <R-Engine/R-EngineExport.hpp>
#include <R-Engine/Maths/Vec.hpp>
#include <R-Engine/Types.hpp>

#include <string>
#include <vector>
#include <R-Engine/ECS/Entity.hpp>

namespace r {

struct R_ENGINE_API UiText {
    std::string value;
    u32 size = 16;
};

struct R_ENGINE_API UiRectSize {
    Vec2f size = {0.f, 0.f};
};

struct R_ENGINE_API UiPosition {
    Vec2f pos = {0.f, 0.f};
};

struct R_ENGINE_API UiTextColor {
    unsigned char r = 0, g = 0, b = 0, a = 255;
};

struct R_ENGINE_API UiColor {
    unsigned char r = 255, g = 255, b = 255, a = 255;
};

struct R_ENGINE_API UiBorderColor {
    unsigned char r = 0, g = 0, b = 0, a = 255;
};

struct R_ENGINE_API UiBorderThickness {
    f32 value = 0.f;
};

struct R_ENGINE_API UiBorderRadius {
    f32 value = 0.f;
};

struct R_ENGINE_API UiZIndex {
    i32 value = 0;
};

struct R_ENGINE_API UiOriginalColor {
    unsigned char r = 0, g = 0, b = 0, a = 255;
};

struct R_ENGINE_API UiOnClickQuit {
};

struct R_ENGINE_API UiTheme {
    f32 hover_dark_percent = 0.5f;
    f32 hover_speed = 14.f;
    f32 restore_speed = 16.f;
    f32 flash_percent = 1.0f;
    f32 flash_speed = 28.f;
    f32 quit_delay = 0.25f;
};

struct R_ENGINE_API UiButton {
};

struct R_ENGINE_API UiButtonState {
    bool hovered = false;
    bool pressed = false;
};

/* New unified interaction component (similar to Bevy's `Interaction`). */
/* Retained alongside UiButtonState for backward compatibility. Systems will */
/* update both when present. Prefer using UiInteraction for new code. */
enum class UiInteractionState : unsigned char { None = 0, Hovered, Pressed };

struct R_ENGINE_API UiInteraction {
    UiInteractionState state = UiInteractionState::None;
};

/* Internal resource replacing previous static globals controlling delayed quit. */
struct R_ENGINE_API UiPendingQuit {
    bool  active = false;
    float timer  = 0.f; /* seconds remaining before quitting */
};

/* ------------------------------------------------------------------------- */
/* Layout / Hierarchy (passive stubs for now; real layout awaits ECS changes) */
/* ------------------------------------------------------------------------- */

/* Basic node data distinct from render rect size (could later store min/max, etc.). */
struct R_ENGINE_API UiNode {
    Vec2f size = {0.f, 0.f};
    f32 padding = 0.f;      /* uniform padding (future use) */
    f32 spacing = 4.f;      /* vertical spacing between children */
};

/* Marker indicating this entity participates as a parent in a layout hierarchy. */
struct R_ENGINE_API UiParent { };

/* Explicit child list (manual for now). Must be filled user-side until ECS exposes entity IDs in queries. */
struct R_ENGINE_API UiChildren {
    std::vector<ecs::Entity> list; /* maintained manually */
};

/* ------------------------------------------------------------------------- */
/* Click Event Bus (lightweight)                                             */
/* ------------------------------------------------------------------------- */

enum class UiClickEventType : unsigned char { Generic, Quit };

struct R_ENGINE_API UiClickEvent {
    UiClickEventType type = UiClickEventType::Generic;
    std::string label; /* optional button text snapshot */
};

struct R_ENGINE_API UiClickEvents {
    std::vector<UiClickEvent> events; /* cleared each frame */
};

/* ------------------------------------------------------------------------- */
/* Pseudo Bundle Helper (manual expansion)                                   */
/* ------------------------------------------------------------------------- */

struct R_ENGINE_API UiButtonBundle {
    UiButton button = {};
    UiButtonState state = {};
    UiOriginalColor original_color = {90, 140, 255, 255};
    UiColor color = {90, 140, 255, 255};
    UiRectSize rect = { {220.f, 70.f} };
    UiBorderRadius radius = {18.f};
    UiBorderThickness thickness = {3.f};
    UiBorderColor border = {30, 50, 120, 255};
    UiZIndex zindex = { 11 };
    UiText text = UiText{ .value = "Button", .size = 32 };
    UiTextColor text_color = {255,255,255,255};
};

/* Helper to spawn a button using the pseudo-bundle. Position provided separately. */
namespace ecs { struct Commands; }

inline ecs::Entity spawn_ui_button(ecs::Commands &commands, const UiButtonBundle &bundle, UiPosition pos);

class R_ENGINE_API UiPlugin final : public Plugin
{
    public:
        void build(Application &app) override;
};

} /* namespace r */

/* Inline helper implementation (requires Command.hpp includes in translation units using it). */
#include <R-Engine/ECS/Command.hpp>

inline r::ecs::Entity r::spawn_ui_button(r::ecs::Commands &commands, const UiButtonBundle &b, r::UiPosition pos)
{
    auto ec = commands.spawn(
        b.button,
        b.state,
        b.original_color,
        b.color,
        b.rect,
        pos,
        b.radius,
        b.thickness,
        b.border,
        b.zindex,
        b.text,
        b.text_color
    );
    return ec.id();
}
