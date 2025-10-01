#pragma once

#include <R-Engine/Plugins/Plugin.hpp>
#include <R-Engine/R-EngineExport.hpp>
#include <R-Engine/Maths/Vec.hpp>
#include <R-Engine/Types.hpp>

#include <string>
#include <vector>
#include <R-Engine/ECS/Entity.hpp>
#include <unordered_map>

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
    f32 press_flash_percent = 1.15f;
    f32 disabled_alpha = 0.4f;
};

/** @brief Marker component tagging an entity as a button. */
struct R_ENGINE_API UiButton { };

/** @brief Legacy button state (retained for backward compatibility). */
struct R_ENGINE_API UiButtonState {
    bool hovered = false;
    bool pressed = false;
};

/** @brief High-level pointer interaction phases. */
enum class UiInteractionState : unsigned char { None = 0, Hovered, Pressed };

/** @brief Runtime interaction snapshot (diffed each frame to emit events). */
struct R_ENGINE_API UiInteraction {
    UiInteractionState state = UiInteractionState::None;
    UiInteractionState previous = UiInteractionState::None;
};

/** @brief Internal state machine for delayed quit button action. */
struct R_ENGINE_API UiPendingQuit {
    bool  active = false;
    float timer  = 0.f;
};


/** @brief Layout stub node (future expansion: constraints, min/max). */
struct R_ENGINE_API UiNode {
    Vec2f size = {0.f, 0.f};
    f32 padding = 0.f;
    f32 spacing = 4.f;
};

/** @brief Marker: entity acts as a layout parent. */
struct R_ENGINE_API UiParent { };

/** @brief Explicit child entity list (manually maintained until hierarchy APIs exist). */
struct R_ENGINE_API UiChildren {
    std::vector<ecs::Entity> list;
};


/** @brief (Deprecated) legacy click event classification. */
enum class UiClickEventType : unsigned char { Generic, Quit };

/* ------------------------------------------------------------------------- */
/* Unified Event Bus                                                         */
/* ------------------------------------------------------------------------- */

/** @brief Unified high-level UI events produced from interaction diffs. */
enum class UiEventType : unsigned char {
    HoverEnter,
    HoverLeave,
    Pressed,
    Released,
    Click,
    QuitClick,
    ValueChanged,
    FocusEnter,
    FocusLeave,
    PointerMove,
    DragStart,
    DragEnd
};

/** @brief Single UI event instance (label optional, filled when text component present). */
struct R_ENGINE_API UiEvent {
    UiEventType type = UiEventType::Click;
    std::string label;
};

/** @brief Double-buffered event storage (current frame + previous frame). */
struct R_ENGINE_API UiEvents {
    std::vector<UiEvent> current;
    std::vector<UiEvent> previous;
};

/* Backward compatibility typedef (old name). */
using UiClickEvent = UiEvent;
using UiClickEvents = UiEvents;

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

/**
 * @brief Per-widget style parameters and animation targets.
 * @details If a field like hover_dark_percent or flash_percent is >= 0 it overrides the global theme.
 * Set disabled = true to fully suppress interaction & events.
 */
struct R_ENGINE_API UiStyle {
    unsigned char base_r = 90, base_g = 140, base_b = 255, base_a = 255;
    f32 hover_dark_percent = 0.5f;
    f32 flash_percent = 1.0f;
    bool disabled = false;
    unsigned char target_r = base_r, target_g = base_g, target_b = base_b, target_a = base_a;
};

/** @brief Dirty flags (style currently used, layout reserved for future). */
struct R_ENGINE_API UiDirty {
    bool style = true;
    bool layout = false;
};


/** @brief Axis for stack layout. */
enum class UiStackAxis : unsigned char { Horizontal, Vertical };

/** @brief Alignment for secondary axis in stack layout. */
enum class UiStackAlign : unsigned char { Start, Center };

/** @brief Stack layout container. */
struct R_ENGINE_API UiStackLayout {
    UiStackAxis axis = UiStackAxis::Vertical;
    UiStackAlign align = UiStackAlign::Center;
    f32 padding = 8.f;
    f32 spacing = 8.f;
};

/** @brief Size constraint: minimum. */
struct R_ENGINE_API UiMinSize { Vec2f value {0.f, 0.f}; };
/** @brief Size constraint: maximum. Use large values to mean 'unbounded'. */
struct R_ENGINE_API UiMaxSize { Vec2f value { 1e9f, 1e9f }; };
/** @brief Preferred size produced by layout / autosize (advisory). */
struct R_ENGINE_API UiPreferredSize { Vec2f value {0.f, 0.f}; };

/** @brief Marker: automatically measure text to set size (if zero) + preferred size. */
struct R_ENGINE_API UiAutoSizeText { };

/** @brief Marker: entity can receive keyboard focus via Tab. */
struct R_ENGINE_API UiFocusable { };
/** @brief Runtime focus flag (redundant but convenient for styling). */
struct R_ENGINE_API UiFocusState { bool focused = false; };

/** @brief Focus navigation context resource (current + ordering). */
struct R_ENGINE_API UiFocusContext {
    ecs::Entity current = 0;
    ecs::Entity previous = 0;
    std::vector<ecs::Entity> order;
};

/** @brief Marker: clip (scissor) all descendants listed in UiChildren to this rect. */
struct R_ENGINE_API UiClipChildren { };
/** @brief Applied clipping rectangle (local absolute space). */
struct R_ENGINE_API UiClipRect { Vec2f pos{0.f,0.f}; Vec2f size{0.f,0.f}; };

/** @brief Scale (animated) applied at render time (does not affect layout). */
struct R_ENGINE_API UiScale { f32 value = 1.f; f32 target = 1.f; };


/** @brief Unified pointer input (mouse + gamepad virtual cursor). */
struct R_ENGINE_API UiInputState {
    Vec2f pointer_pos {0.f,0.f};
    Vec2f prev_pointer_pos {0.f,0.f};
    bool pointer_down = false;
    bool pointer_pressed = false;
    bool pointer_released = false;
    bool any_gamepad = false;
};

/** @brief Config for virtual cursor input & mappings. */
struct R_ENGINE_API UiInputConfig {
    f32 gamepad_cursor_speed = 600.f;
    int gamepad_index = 0;
    int confirm_button = 0;
};

/** @brief Simple LRU-like cache entry for text measurement. */
struct R_ENGINE_API UiTextMeasureCacheEntry {
    std::string key;
    int width = 0;
    u64 last_used_frame = 0;
};

/** @brief Cache storing measured text widths to avoid repeated MeasureText calls. */
struct R_ENGINE_API UiTextMeasureCache {
    std::vector<UiTextMeasureCacheEntry> entries;
    u32 capacity = 256;
};

/** @brief Advanced logger configuration with throttle per event type (ms). */
struct R_ENGINE_API UiAdvancedLoggerConfig {
    bool enabled = false;
    u32 throttle_hover_ms = 0;
    u32 throttle_press_ms = 0;
    u32 throttle_release_ms = 0;
    u32 throttle_click_ms = 0;
    u32 throttle_focus_ms = 0;
    u32 throttle_pointer_ms = 0;
    u32 throttle_drag_ms = 0;
    /* internal last log times */
    double last_hover_time = -1.0;
    double last_press_time = -1.0;
    double last_release_time = -1.0;
    double last_click_time = -1.0;
    double last_focus_time = -1.0;
    double last_pointer_time = -1.0;
    double last_drag_time = -1.0;
};

/** @brief Animation target property kinds (extendable). */
enum class UiAnimProperty : unsigned char { Scale, Alpha };

/** @brief Declarative float animation track for a property (one-shot or looping). */
struct R_ENGINE_API UiAnimFloat {
    UiAnimProperty property = UiAnimProperty::Scale;
    f32 from = 1.f;
    f32 to = 1.f;
    f32 duration = 0.2f; /* seconds */
    f32 elapsed = 0.f;
    bool playing = false;
    bool loop = false;
};

/** @brief Triggers for starting animations on events. */
struct R_ENGINE_API UiAnimOnHover { bool start = true; };
struct R_ENGINE_API UiAnimOnClick { bool start = true; };
struct R_ENGINE_API UiAnimOnFocus { bool start = true; };

/** @brief Per-entity animation set (multiple tracks). */
struct R_ENGINE_API UiAnimations { std::vector<UiAnimFloat> tracks; };

/** @brief Profiler config + rolling samples of timings. */
struct R_ENGINE_API UiProfiler {
    u32 max_samples = 120;
    std::vector<double> interaction_samples;
    std::vector<double> style_samples;
    std::vector<double> render_rect_samples;
    std::vector<double> render_text_samples;
    std::vector<double> animation_samples;
    std::vector<double> input_samples;
};


/** @brief Named entity for lookup. */
struct R_ENGINE_API UiName { std::string value; };

/** @brief Global registry mapping names to entities (rebuilt each frame). */
struct R_ENGINE_API UiNameRegistry { std::unordered_map<std::string, ecs::Entity> map; };

/** @brief Checkbox / toggle state. */
struct R_ENGINE_API UiToggle { bool value = false; };

/** @brief Slider (horizontal) floating value. */
struct R_ENGINE_API UiSlider {
    float value = 0.f;
    float min = 0.f;
    float max = 1.f;
    bool dragging = false;
};

/** @brief Marker: label text bound to a slider's value (formatted each frame). */
struct R_ENGINE_API UiLabelBindSlider { ecs::Entity slider = 0; };

/** @brief Virtual list of text items with windowing (single column). */
struct R_ENGINE_API UiVirtualList {
    std::vector<std::string> items;
    i32 first_visible = 0;
    i32 visible_count = 0;
    f32 item_height = 20.f;
    std::vector<ecs::Entity> item_entities;
};

/** @brief Marker: style should inherit a base color (static copy). */
struct R_ENGINE_API UiStyleInherit { };

/** @brief Stored inherited base RGBA for style inheritance. */
struct R_ENGINE_API UiInheritedBaseColor { unsigned char r=0,g=0,b=0,a=255; };

/** @brief Internal cache of slider values for label binding. */
struct R_ENGINE_API UiSliderValueCache { std::vector<std::pair<ecs::Entity,float>> values; };

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
    /* Phase 3 additions */
    u32 draw_calls = 0; /* after batching */
    u32 text_cache_hits = 0;
    u32 text_cache_misses = 0;
    double input_ms = 0.0;
    double animation_ms = 0.0;
};

/** @brief Marker: entity participates in drag detection. */
struct R_ENGINE_API UiDraggable { };
/** @brief Per-entity drag runtime state. */
struct R_ENGINE_API UiDragState { bool dragging=false; Vec2f start_pos{0.f,0.f}; };

/** @brief Timeline animation keyframe (time,value). */
struct R_ENGINE_API UiTimelineKeyframe { float time=0.f; float value=0.f; };
/** @brief Timeline property kinds. */
enum class UiTimelineProperty : unsigned char { Scale, Alpha };
/** @brief Timeline track for one property. */
struct R_ENGINE_API UiTimelineTrack { UiTimelineProperty property=UiTimelineProperty::Scale; std::vector<UiTimelineKeyframe> keys; bool loop=false; };
/** @brief Per-entity timeline animation set. */
struct R_ENGINE_API UiTimeline { bool playing=false; float time=0.f; std::vector<UiTimelineTrack> tracks; };

/** @brief Headless execution configuration (tests). */
struct R_ENGINE_API UiHeadlessConfig { bool enabled=false; };
/** @brief Scripted headless input event. */
struct R_ENGINE_API UiHeadlessEvent { float dt=0.016f; Vec2f pointer_pos{0.f,0.f}; bool press=false; bool release=false; };
/** @brief Sequence of headless input events consumed each frame. */
struct R_ENGINE_API UiHeadlessScript { std::vector<UiHeadlessEvent> events; size_t index=0; };

/** @brief Load theme values from a simple key=value text file. */
bool R_ENGINE_API load_ui_theme_file(class Application &app, const std::string &path);

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
    UiInteraction interaction = {};
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
