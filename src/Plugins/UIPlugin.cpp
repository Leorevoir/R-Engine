/* New refactored UiPlugin implementation inspired by Bevy's UiPlugin layout. */
/* We split responsibilities into: interaction update, visual style (color */
/* transitions), action handling (quit), and rendering. We keep previous */
/* behavior while introducing UiInteraction + UiPendingQuit resource. */

#include <R-Engine/Plugins/UIPlugin.hpp>
#include <R-Engine/Application.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/Core/FrameTime.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <algorithm>
#include <cstring>

namespace r {

/* ---------------------------------------------------------------------------- */
/* Rendering helpers (unchanged logic, reorganized)                          */
/* ---------------------------------------------------------------------------- */

struct _UiRenderableText { UiPosition *pos; UiText *text; UiTextColor *color; UiZIndex *z; UiRectSize *rect_size; };
struct _UiRenderableRect { UiPosition *pos; UiRectSize *size; UiColor *color; UiBorderColor *border_color; UiBorderThickness *border_thickness; UiBorderRadius *radius; UiZIndex *z; };

/* --- helpers: generic sorting by z --------------------------------------- */
template<typename T>
static void _ui_sort_by_z(std::vector<T> &items)
{
    std::sort(items.begin(), items.end(), [] (const T &a, const T &b) {
        const i32 za = a.z ? a.z->value : 0;
        const i32 zb = b.z ? b.z->value : 0;
        return za < zb;
    });
}

/* --- text helpers --------------------------------------------------------- */
static void _ui_collect_text_items(ecs::Query<ecs::Mut<UiText>, ecs::Mut<UiPosition>, ecs::Ref<UiTextColor>, ecs::Optional<UiZIndex>, ecs::Optional<UiRectSize>> &q, std::vector<_UiRenderableText> &out)
{
    for (auto [text_w, pos_w, col_w, z_w, rect_w] : q) {
        out.push_back({pos_w.ptr, text_w.ptr, const_cast<UiTextColor *>(col_w.ptr), const_cast<UiZIndex *>(z_w.ptr), const_cast<UiRectSize *>(rect_w.ptr)});
    }
}

static void _ui_draw_text_items(const std::vector<_UiRenderableText> &items)
{
    for (auto &it : items) {
        const Color c = {it.color->r, it.color->g, it.color->b, it.color->a};
        float draw_x = it.pos->pos.x;
        float draw_y = it.pos->pos.y;
        if (it.rect_size) {
            const int text_w_px_int = MeasureText(it.text->value.c_str(), static_cast<int>(it.text->size));
            const float text_w_px = static_cast<float>(text_w_px_int);
            draw_x += (it.rect_size->size.x - text_w_px) * 0.5f;
            draw_y += (it.rect_size->size.y - static_cast<float>(it.text->size)) * 0.5f;
        }
        DrawText(it.text->value.c_str(), static_cast<int>(draw_x), static_cast<int>(draw_y), static_cast<int>(it.text->size), c);
    }
}

static void _ui_render_text_system(ecs::Query<ecs::Mut<UiText>, ecs::Mut<UiPosition>, ecs::Ref<UiTextColor>, ecs::Optional<UiZIndex>, ecs::Optional<UiRectSize>> q)
{
    std::vector<_UiRenderableText> items; items.reserve(64);
    _ui_collect_text_items(q, items);
    _ui_sort_by_z(items);
    _ui_draw_text_items(items);
}

/* --- rect helpers --------------------------------------------------------- */
static void _ui_collect_rect_items(ecs::Query<ecs::Mut<UiRectSize>, ecs::Mut<UiPosition>, ecs::Ref<UiColor>, ecs::Optional<UiBorderColor>, ecs::Optional<UiBorderThickness>, ecs::Optional<UiBorderRadius>, ecs::Optional<UiZIndex>> &q, std::vector<_UiRenderableRect> &out)
{
    for (auto [size_w, pos_w, col_w, bcol_w, bthick_w, radius_w, z_w] : q) {
        out.push_back({pos_w.ptr, size_w.ptr, const_cast<UiColor *>(col_w.ptr), const_cast<UiBorderColor *>(bcol_w.ptr), const_cast<UiBorderThickness *>(bthick_w.ptr), const_cast<UiBorderRadius *>(radius_w.ptr), const_cast<UiZIndex *>(z_w.ptr)});
    }
}

static void _ui_draw_rect_items(const std::vector<_UiRenderableRect> &items)
{
    for (auto &it : items) {
        const Rectangle rect = {it.pos->pos.x, it.pos->pos.y, it.size->size.x, it.size->size.y};
        const Color c = {it.color->r, it.color->g, it.color->b, it.color->a};
        const f32 radius_value = it.radius ? it.radius->value : 0.f;
        const f32 thickness = it.border_thickness ? it.border_thickness->value : 0.f;
        if (radius_value > 0.f) {
            DrawRectangleRounded(rect, radius_value / std::max(rect.width, rect.height), 8, c);
        } else {
            DrawRectangleRec(rect, c);
        }
        if (thickness > 0.f) {
            const Color bc = it.border_color ? Color{it.border_color->r, it.border_color->g, it.border_color->b, it.border_color->a} : Color{0,0,0,255};
            if (radius_value > 0.f) {
                DrawRectangleRoundedLinesEx(rect, radius_value / std::max(rect.width, rect.height), 8, thickness, bc);
            } else {
                DrawRectangleLinesEx(rect, thickness, bc);
            }
        }
    }
}

static void _ui_render_rect_system(ecs::Query<ecs::Mut<UiRectSize>, ecs::Mut<UiPosition>, ecs::Ref<UiColor>, ecs::Optional<UiBorderColor>, ecs::Optional<UiBorderThickness>, ecs::Optional<UiBorderRadius>, ecs::Optional<UiZIndex>> q)
{
    std::vector<_UiRenderableRect> items; items.reserve(64);
    _ui_collect_rect_items(q, items);
    _ui_sort_by_z(items);
    _ui_draw_rect_items(items);
}

/* ---------------------------------------------------------------------------- */
/* Interaction system (updates UiButtonState & UiInteraction)                  */
/* ---------------------------------------------------------------------------- */

struct _UiPointerState { Vector2 pos; bool down; bool pressed; bool released; };

static _UiPointerState _ui_pointer_state()
{
    return { GetMousePosition(), IsMouseButtonDown(MOUSE_BUTTON_LEFT), IsMouseButtonPressed(MOUSE_BUTTON_LEFT), IsMouseButtonReleased(MOUSE_BUTTON_LEFT) };
}

static void _ui_process_one_interaction(const _UiPointerState &ptr, UiPosition *pos, UiRectSize *size, UiButtonState *legacy, UiInteraction *interaction)
{
    const Rectangle rect = {pos->pos.x, pos->pos.y, size->size.x, size->size.y};
    const bool inside = ptr.pos.x >= rect.x && ptr.pos.x <= rect.x + rect.width && ptr.pos.y >= rect.y && ptr.pos.y <= rect.y + rect.height;
    bool hovered = inside;
    bool pressed_logic = false;
    if (legacy) {
        legacy->hovered = hovered;
        if (inside && ptr.pressed) legacy->pressed = true;
        if (legacy->pressed && ptr.released) legacy->pressed = false;
        if (!ptr.down && !inside && !legacy->hovered) legacy->pressed = false;
        pressed_logic = legacy->pressed && inside && ptr.down;
    } else {
        pressed_logic = inside && ptr.down;
    }
    if (interaction) {
        if (pressed_logic) interaction->state = UiInteractionState::Pressed;
        else if (hovered) interaction->state = UiInteractionState::Hovered;
        else interaction->state = UiInteractionState::None;
    }
}

static void _ui_interaction_system(ecs::Query<ecs::Mut<UiPosition>, ecs::Mut<UiRectSize>, ecs::Optional<UiButton>, ecs::Optional<UiButtonState>, ecs::Optional<UiInteraction>> q)
{
    const _UiPointerState ptr = _ui_pointer_state();
    for (auto [pos_w, size_w, button_w, state_w, interaction_w] : q) {
        if (!button_w.ptr) continue;
        if (!state_w.ptr && !interaction_w.ptr) continue;
        _ui_process_one_interaction(ptr, pos_w.ptr, size_w.ptr, const_cast<UiButtonState *>(state_w.ptr), const_cast<UiInteraction *>(interaction_w.ptr));
    }
}

/* ---------------------------------------------------------------------------- */
/* Visual style (color transitions) - adapted from previous button logic      */
/* ---------------------------------------------------------------------------- */

static unsigned char _ui_lerp_u8(unsigned char from, unsigned char to, f32 spd)
{
    f32 f = static_cast<f32>(from);
    f32 t = static_cast<f32>(to);
    f32 v = f + (t - f) * std::clamp(spd * 0.016f, 0.f, 1.f);
    return static_cast<unsigned char>(std::clamp(static_cast<int>(v), 0, 255));
}

static void _ui_apply_hover_flash(UiColor *col, const UiOriginalColor *orig, bool hovered, bool pressed, bool just_pressed, const UiTheme *theme)
{
    const f32 target_dark_factor = 1.f - std::clamp(theme->hover_dark_percent, 0.f, 0.95f);
    const f32 flash_factor = 1.f + std::clamp(theme->flash_percent, 0.f, 1.f);
    const int dark_r = static_cast<int>(orig->r * target_dark_factor);
    const int dark_g = static_cast<int>(orig->g * target_dark_factor);
    const int dark_b = static_cast<int>(orig->b * target_dark_factor);
    const int flash_r = static_cast<int>(std::min<f32>(255.f, orig->r * flash_factor));
    const int flash_g = static_cast<int>(std::min<f32>(255.f, orig->g * flash_factor));
    const int flash_b = static_cast<int>(std::min<f32>(255.f, orig->b * flash_factor));
    if (hovered && !pressed) {
        col->r = _ui_lerp_u8(col->r, static_cast<unsigned char>(dark_r), theme->hover_speed);
        col->g = _ui_lerp_u8(col->g, static_cast<unsigned char>(dark_g), theme->hover_speed);
        col->b = _ui_lerp_u8(col->b, static_cast<unsigned char>(dark_b), theme->hover_speed);
    } else if (!pressed) {
        col->r = _ui_lerp_u8(col->r, orig->r, theme->restore_speed);
        col->g = _ui_lerp_u8(col->g, orig->g, theme->restore_speed);
        col->b = _ui_lerp_u8(col->b, orig->b, theme->restore_speed);
    }
    if (hovered && just_pressed) {
        col->r = _ui_lerp_u8(col->r, static_cast<unsigned char>(flash_r), theme->flash_speed);
        col->g = _ui_lerp_u8(col->g, static_cast<unsigned char>(flash_g), theme->flash_speed);
        col->b = _ui_lerp_u8(col->b, static_cast<unsigned char>(flash_b), theme->flash_speed);
    }
}

static void _ui_button_color_system(ecs::Query<ecs::Mut<UiColor>, ecs::Ref<UiOriginalColor>, ecs::Optional<UiButtonState>, ecs::Optional<UiInteraction>> q, ecs::Res<UiTheme> theme)
{
    const bool mouse_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    for (auto [color_w, original_w, state_w, interaction_w] : q) {
        const UiOriginalColor *orig = original_w.ptr;
        const UiButtonState *legacy = state_w.ptr;
        const UiInteraction *interaction = interaction_w.ptr;
        bool hovered = false, pressed = false;
        if (legacy) { hovered = legacy->hovered; pressed = legacy->pressed; }
        if (interaction) { hovered = hovered || interaction->state == UiInteractionState::Hovered || interaction->state == UiInteractionState::Pressed; pressed = pressed || interaction->state == UiInteractionState::Pressed; }
        _ui_apply_hover_flash(color_w.ptr, orig, hovered, pressed, mouse_pressed, theme.ptr);
    }
}

/* ---------------------------------------------------------------------------- */
/* Action system (quit button)                                                */
/* ---------------------------------------------------------------------------- */

static void _ui_button_action_system(ecs::Query<ecs::Ref<UiButton>, ecs::Optional<UiOnClickQuit>, ecs::Optional<UiButtonState>, ecs::Optional<UiInteraction>> q,
                                     ecs::Res<UiTheme> theme, ecs::Res<UiPendingQuit> pending, ecs::Res<UiClickEvents> events)
{
    const bool mouse_released = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    for (auto [button_w, quit_w, state_w, interaction_w] : q) {
        if (!quit_w.ptr) continue; /* only interested in quit buttons here */

        bool trigger = false;
        if (state_w.ptr) {
            /* Legacy: a release inside while previously pressed */
            /* (Interaction release detection already covered by state machine above) */
            trigger = mouse_released && state_w.ptr->hovered; /* approximate previous logic */
        }
        if (interaction_w.ptr) {
            /* If interaction was pressed and mouse released inside (approx). We only have current state, so rely on release + hovered. */
            if (mouse_released && (interaction_w.ptr->state == UiInteractionState::Hovered || interaction_w.ptr->state == UiInteractionState::Pressed)) {
                trigger = true;
            }
        }

        if (trigger) {
            if (theme.ptr->quit_delay <= 0.f) {
                r::Application::quit = true;
            } else {
                auto *res = const_cast<UiPendingQuit *>(pending.ptr);
                res->active = true;
                res->timer = theme.ptr->quit_delay;
            }
            /* Emit click event (Quit type) */
            auto *ev = const_cast<UiClickEvents *>(events.ptr);
            ev->events.push_back(UiClickEvent{UiClickEventType::Quit, "Quit"});
        }
    }
}

static void _ui_pending_quit_system(ecs::Res<core::FrameTime> ft, ecs::Res<UiPendingQuit> pending)
{
    if (pending.ptr->active) {
        auto *res = const_cast<UiPendingQuit *>(pending.ptr);
        res->timer -= ft.ptr->delta_time;
        if (res->timer <= 0.f) {
            r::Application::quit = true;
            res->active = false;
        }
    }
}

/* ---------------------------------------------------------------------------- */
/* Event clear system (runs each frame before interactions)                     */
/* ---------------------------------------------------------------------------- */
static void _ui_click_events_clear_system(ecs::Res<UiClickEvents> events)
{
    auto *ev = const_cast<UiClickEvents *>(events.ptr);
    ev->events.clear();
}

/* ---------------------------------------------------------------------------- */
/* Layout stub system (placeholder - no hierarchy logic yet)                   */
/* ---------------------------------------------------------------------------- */
static void _ui_layout_stub_system(ecs::Query<ecs::Mut<UiPosition>, ecs::Optional<UiNode>> q)
{
    /* Intentionally does nothing for now; positions remain as authored. */
    (void)q;
}

/* ---------------------------------------------------------------------------- */
/* Plugin build                                                               */
/* ---------------------------------------------------------------------------- */

void UiPlugin::build(Application &app)
{
    app
        .insert_resource(UiTheme{})
        .insert_resource(UiPendingQuit{})
        .insert_resource(UiClickEvents{})
        /* Update: interaction, visuals, action, delayed quit */
        .add_systems(Schedule::UPDATE,
            _ui_click_events_clear_system,
            _ui_layout_stub_system,
            _ui_interaction_system,
            _ui_button_color_system,
            _ui_button_action_system,
            _ui_pending_quit_system)
        /* Render */
        .add_systems(Schedule::RENDER, _ui_render_rect_system, _ui_render_text_system);
}

} /* namespace r */
