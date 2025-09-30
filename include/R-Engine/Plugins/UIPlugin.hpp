/**
 * @file UIPlugin.hpp
 * @brief User Interface (UI) plugin public API (Phase 1 foundation).
 * @details Provides ECS components, resources and a plugin that implement:
 *  - Unified event bus (hover / press / release / click / quit)
 *  - Filterable event logger
 *  - Style system (per-widget style + global theme + disabled state)
 *  - Frame statistics (counts + timings)
 *  - Pseudo bundle helper for spawning buttons quickly
 *  - Forward-looking stubs for layout / hierarchy (not yet active)
 * @note Layout & ValueChanged events are placeholders for future phases.
 */
#pragma once

#include <R-Engine/Plugins/Plugin.hpp>
#include <R-Engine/R-EngineExport.hpp>
#include <R-Engine/Maths/Vec.hpp>
#include <R-Engine/Types.hpp>

#include <string>
#include <vector>
#include <R-Engine/ECS/Entity.hpp>

namespace r {

/** @brief Text label rendered inside a UI element. */
struct R_ENGINE_API UiText {
    std::string value;
    u32 size = 16;
};

/** @brief Rectangle (widget) size in pixels. */
struct R_ENGINE_API UiRectSize {
    Vec2f size = {0.f, 0.f};
};

/** @brief Absolute top-left pixel position of a UI element. */
struct R_ENGINE_API UiPosition {
    Vec2f pos = {0.f, 0.f};
};

/** @brief Text color (RGBA). */
struct R_ENGINE_API UiTextColor {
    unsigned char r = 0, g = 0, b = 0, a = 255;
};

/** @brief Current fill color (animated towards style target). */
struct R_ENGINE_API UiColor {
    unsigned char r = 255, g = 255, b = 255, a = 255;
};

/** @brief Border stroke color. */
struct R_ENGINE_API UiBorderColor {
    unsigned char r = 0, g = 0, b = 0, a = 255;
};

/** @brief Border stroke thickness in pixels. */
struct R_ENGINE_API UiBorderThickness {
    f32 value = 0.f;
};

/** @brief Rounded corner radius. */
struct R_ENGINE_API UiBorderRadius {
    f32 value = 0.f;
};

/** @brief Z layering order (lower renders first). */
struct R_ENGINE_API UiZIndex {
    i32 value = 0;
};

/** @brief Original (base) color used as style source (never animated). */
struct R_ENGINE_API UiOriginalColor {
    unsigned char r = 0, g = 0, b = 0, a = 255;
};

/** @brief Marker: clicking this button schedules / triggers application quit. */
struct R_ENGINE_API UiOnClickQuit { };

/**
 * @brief Global theme parameters applied to all styled widgets.
 * @details Per-widget overrides in UiStyle take precedence when >= 0.
 */
struct R_ENGINE_API UiTheme {
    f32 hover_dark_percent = 0.5f;
    f32 hover_speed = 14.f;
    f32 restore_speed = 16.f;
    f32 flash_percent = 1.0f;
    f32 flash_speed = 28.f;
    f32 quit_delay = 0.25f;
    /* New global style knobs */
    f32 press_flash_percent = 1.15f; /* multiplicative brighten when pressed */
    f32 disabled_alpha = 0.4f;       /* alpha multiplier for disabled */
};

/** @brief Marker component tagging an entity as a button. */
struct R_ENGINE_API UiButton { };

/** @brief Legacy button state (retained for backward compatibility). */
struct R_ENGINE_API UiButtonState {
    bool hovered = false;
    bool pressed = false;
};

/* New unified interaction component (similar to Bevy's `Interaction`). */
/* Retained alongside UiButtonState for backward compatibility. Systems will */
/* update both when present. Prefer using UiInteraction for new code. */
/** @brief High-level pointer interaction phases. */
enum class UiInteractionState : unsigned char { None = 0, Hovered, Pressed };

/** @brief Runtime interaction snapshot (diffed each frame to emit events). */
struct R_ENGINE_API UiInteraction {
    UiInteractionState state = UiInteractionState::None;
    UiInteractionState previous = UiInteractionState::None; /* preserved last frame for event diff */
};

/* Internal resource replacing previous static globals controlling delayed quit. */
/** @brief Internal state machine for delayed quit button action. */
struct R_ENGINE_API UiPendingQuit {
    bool  active = false;
    float timer  = 0.f; /* seconds remaining before quitting */
};

/* ------------------------------------------------------------------------- */
/* Layout / Hierarchy (passive stubs for now; real layout awaits ECS changes) */
/* ------------------------------------------------------------------------- */

/* Basic node data distinct from render rect size (could later store min/max, etc.). */
/** @brief Layout stub node (future expansion: constraints, min/max). */
struct R_ENGINE_API UiNode {
    Vec2f size = {0.f, 0.f};
    f32 padding = 0.f;      /* uniform padding (future use) */
    f32 spacing = 4.f;      /* vertical spacing between children */
};

/* Marker indicating this entity participates as a parent in a layout hierarchy. */
/** @brief Marker: entity acts as a layout parent. */
struct R_ENGINE_API UiParent { };

/* Explicit child list (manual for now). Must be filled user-side until ECS exposes entity IDs in queries. */
/** @brief Explicit child entity list (manually maintained until hierarchy APIs exist). */
struct R_ENGINE_API UiChildren {
    std::vector<ecs::Entity> list; /* maintained manually */
};

/* ------------------------------------------------------------------------- */
/* Click Event Bus (lightweight)                                             */
/* ------------------------------------------------------------------------- */

/** @brief (Deprecated) legacy click event classification. */
enum class UiClickEventType : unsigned char { Generic, Quit };

/* ------------------------------------------------------------------------- */
/* Unified Event Bus                                                         */
/* ------------------------------------------------------------------------- */

/* High level UI event kinds (extensible). */
/** @brief Unified high-level UI events produced from interaction diffs. */
enum class UiEventType : unsigned char {
    HoverEnter,
    HoverLeave,
    Pressed,
    Released,
    Click,
    QuitClick,   /* specialized click leading to quit */
    ValueChanged /* reserved for future widgets */
};

/** @brief Single UI event instance (label optional, filled when text component present). */
struct R_ENGINE_API UiEvent {
    UiEventType type = UiEventType::Click;
    std::string label; /* optional textual label (e.g. button text) */
};

/* Double buffered storage so consumers (potentially later in frame) can still */
/* reference last frame events if needed. For now we only use current. */
/** @brief Double-buffered event storage (current frame + previous frame). */
struct R_ENGINE_API UiEvents {
    std::vector<UiEvent> current;
    std::vector<UiEvent> previous;
};

/* Backward compatibility typedef (old name). */
using UiClickEvent = UiEvent; /* deprecated */
using UiClickEvents = UiEvents; /* deprecated */

/* Logger configuration resource allowing filtering. */
/** @brief Runtime filtering flags for the UI event logger system. */
struct R_ENGINE_API UiEventLoggerConfig {
    bool enabled = false;
    bool log_hover = false;
    bool log_press = true;
    bool log_release = true;
    bool log_click = true;
    bool log_quit = true;
};

/* ------------------------------------------------------------------------- */
/* Style system                                                              */
/* ------------------------------------------------------------------------- */

/* Declarative style for a widget. Drives final UiColor. */
/**
 * @brief Per-widget style parameters and animation targets.
 * @details If a field like hover_dark_percent or flash_percent is >= 0 it overrides the global theme.
 * Set disabled = true to fully suppress interaction & events.
 */
struct R_ENGINE_API UiStyle {
    unsigned char base_r = 90, base_g = 140, base_b = 255, base_a = 255;
    f32 hover_dark_percent = 0.5f;  /* overrides global if >= 0 */
    f32 flash_percent = 1.0f;       /* overrides global if >= 0 */
    bool disabled = false;
    /* Internal target color (computed on interaction transitions). */
    unsigned char target_r = base_r, target_g = base_g, target_b = base_b, target_a = base_a;
};

/* Dirty flags allow selective recomputation; kept minimal for now. */
/** @brief Dirty flags (style currently used, layout reserved for future). */
struct R_ENGINE_API UiDirty {
    bool style = true;  /* when true, UiStyle target will be recomputed or animation kept */
    bool layout = false; /* reserved */
};

/* Frame statistics (reset each frame). */
/**
 * @brief Frame statistics for UI subsystem.
 * @details Reset every frame by the frame-begin system. Times in milliseconds.
 */
struct R_ENGINE_API UiStats {
    u64 frame_index = 0;
    u32 events_emitted = 0;
    u32 rects_drawn = 0;
    u32 texts_drawn = 0;
    u32 interaction_entities = 0;
    double interaction_ms = 0.0;
    double style_ms = 0.0;
    double render_rect_ms = 0.0;
    double render_text_ms = 0.0;
};

/* ------------------------------------------------------------------------- */
/* Pseudo Bundle Helper (manual expansion)                                   */
/* ------------------------------------------------------------------------- */

/**
 * @brief Convenience aggregate to spawn a fully configured button.
 * @details Expands into multiple components; modify fields before passing to spawn_ui_button().
 */
struct R_ENGINE_API UiButtonBundle {
    UiButton button = {};
    UiButtonState state = {};
    UiInteraction interaction = {}; /* required for new interaction system */
    UiOriginalColor original_color = {90, 140, 255, 255};
    UiColor color = {90, 140, 255, 255};
    UiStyle style = {};
    UiDirty dirty = {};
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

/**
 * @brief Spawn a button entity with all UI components from a bundle.
 * @param commands Command buffer used to create the entity.
 * @param bundle Pre-filled component aggregate (colors, text, style...).
 * @param pos Position component (top-left coordinates) supplied separately.
 * @return Newly created entity id.
 */
inline ecs::Entity spawn_ui_button(ecs::Commands &commands, const UiButtonBundle &bundle, UiPosition pos);

/** @brief Registers all UI systems & resources into an application. */
class R_ENGINE_API UiPlugin final : public Plugin {
public:
    /** @brief Install resources & systems (update + render schedules). */
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
        b.interaction,
        b.original_color,
        b.color,
        b.style,
        b.dirty,
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
