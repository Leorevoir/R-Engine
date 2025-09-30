/**
 * @file UIPlugin.cpp
 * @brief Implementation of Phase 1 UI systems (interaction, events, style, rendering, logging).
 * @details UPDATE order:
 *  1. _ui_events_frame_begin_system (swap event buffers + reset stats)
 *  2. _ui_layout_stub_system (reserved for future layout pass)
 *  3. _ui_interaction_system (pointer hit-test -> UiInteraction state diff)
 *  4. _ui_interaction_events_system (derive UiEvents from interaction transitions)
 *  5. _ui_style_system (compute target colors + animate)
 *  6. _ui_button_action_system (quit button behavior + QuitClick event)
 *  7. _ui_pending_quit_system (delayed quit countdown)
 *  8. _ui_event_logger_system (optional filtered logging)
 * RENDER order:
 *  - _ui_render_rect_system
 *  - _ui_render_text_system
 */

#include <R-Engine/Plugins/UIPlugin.hpp>
#include <R-Engine/Application.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/Core/FrameTime.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <algorithm>
#include <chrono>

namespace r {

/* Internal utility: scope-based millisecond timer feeding UiStats field. */
struct UiScopedTimerInternal {
    std::chrono::high_resolution_clock::time_point start;
    double *dst;
    explicit UiScopedTimerInternal(double *d) : start(std::chrono::high_resolution_clock::now()), dst(d) {}
    ~UiScopedTimerInternal() { if (dst) *dst = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - start).count(); }
};

/* Batched render items (collected then sorted by z). */
struct UiRenderableTextInternal { UiPosition *pos; UiText *text; UiTextColor *color; UiZIndex *z; UiRectSize *rect_size; };
struct UiRenderableRectInternal { UiPosition *pos; UiRectSize *size; UiColor *color; UiBorderColor *border_color; UiBorderThickness *border_thickness; UiBorderRadius *radius; UiZIndex *z; };

template <typename T>
/** @brief Helper to sort render batches by z-index ascending. */
static void _ui_sort_by_z(std::vector<T> &items) {
    std::sort(items.begin(), items.end(), [](const T &a, const T &b){
        const i32 za = a.z ? a.z->value : 0; const i32 zb = b.z ? b.z->value : 0; return za < zb; });
}

static void _ui_collect_text_items(ecs::Query<ecs::Mut<UiText>, ecs::Mut<UiPosition>, ecs::Ref<UiTextColor>, ecs::Optional<UiZIndex>, ecs::Optional<UiRectSize>> &q, std::vector<UiRenderableTextInternal> &out) {
    for (auto [text_w, pos_w, col_w, z_w, rect_w] : q) out.push_back({pos_w.ptr, text_w.ptr, const_cast<UiTextColor *>(col_w.ptr), const_cast<UiZIndex *>(z_w.ptr), const_cast<UiRectSize *>(rect_w.ptr)});
}

static void _ui_collect_rect_items(ecs::Query<ecs::Mut<UiRectSize>, ecs::Mut<UiPosition>, ecs::Ref<UiColor>, ecs::Optional<UiBorderColor>, ecs::Optional<UiBorderThickness>, ecs::Optional<UiBorderRadius>, ecs::Optional<UiZIndex>> &q, std::vector<UiRenderableRectInternal> &out) {
    for (auto [size_w, pos_w, col_w, bcol_w, bthick_w, radius_w, z_w] : q) out.push_back({pos_w.ptr, size_w.ptr, const_cast<UiColor *>(col_w.ptr), const_cast<UiBorderColor *>(bcol_w.ptr), const_cast<UiBorderThickness *>(bthick_w.ptr), const_cast<UiBorderRadius *>(radius_w.ptr), const_cast<UiZIndex *>(z_w.ptr)});
}

static void _ui_draw_text_items(const std::vector<UiRenderableTextInternal> &items) {
    for (auto &it : items) {
        const Color c = {it.color->r, it.color->g, it.color->b, it.color->a};
        float draw_x = it.pos->pos.x, draw_y = it.pos->pos.y;
        if (it.rect_size) {
            const int wpx = MeasureText(it.text->value.c_str(), (int)it.text->size);
            draw_x += (it.rect_size->size.x - (float)wpx) * 0.5f;
            draw_y += (it.rect_size->size.y - (float)it.text->size) * 0.5f;
        }
        DrawText(it.text->value.c_str(), (int)draw_x, (int)draw_y, (int)it.text->size, c);
    }
}

static void _ui_draw_rect_items(const std::vector<UiRenderableRectInternal> &items) {
    for (auto &it : items) {
        const Rectangle rect = {it.pos->pos.x, it.pos->pos.y, it.size->size.x, it.size->size.y};
        const Color c = {it.color->r, it.color->g, it.color->b, it.color->a};
        const f32 radius_value = it.radius ? it.radius->value : 0.f;
        const f32 thickness = it.border_thickness ? it.border_thickness->value : 0.f;
        if (radius_value > 0.f) DrawRectangleRounded(rect, radius_value / std::max(rect.width, rect.height), 8, c);
        else DrawRectangleRec(rect, c);
        if (thickness > 0.f) {
            const Color bc = it.border_color ? Color{it.border_color->r, it.border_color->g, it.border_color->b, it.border_color->a} : Color{0,0,0,255};
            if (radius_value > 0.f) DrawRectangleRoundedLinesEx(rect, radius_value / std::max(rect.width, rect.height), 8, thickness, bc);
            else DrawRectangleLinesEx(rect, thickness, bc);
        }
    }
}

static void _ui_render_text_system(ecs::Query<ecs::Mut<UiText>, ecs::Mut<UiPosition>, ecs::Ref<UiTextColor>, ecs::Optional<UiZIndex>, ecs::Optional<UiRectSize>> q, ecs::Res<UiStats> stats) {
    UiScopedTimerInternal t(const_cast<double*>(&stats.ptr->render_text_ms));
    std::vector<UiRenderableTextInternal> items; items.reserve(64); _ui_collect_text_items(q, items); _ui_sort_by_z(items); _ui_draw_text_items(items);
    auto *st = const_cast<UiStats*>(stats.ptr); st->texts_drawn += (u32)items.size();
}
static void _ui_render_rect_system(ecs::Query<ecs::Mut<UiRectSize>, ecs::Mut<UiPosition>, ecs::Ref<UiColor>, ecs::Optional<UiBorderColor>, ecs::Optional<UiBorderThickness>, ecs::Optional<UiBorderRadius>, ecs::Optional<UiZIndex>> q, ecs::Res<UiStats> stats) {
    UiScopedTimerInternal t(const_cast<double*>(&stats.ptr->render_rect_ms));
    std::vector<UiRenderableRectInternal> items; items.reserve(64); _ui_collect_rect_items(q, items); _ui_sort_by_z(items); _ui_draw_rect_items(items);
    auto *st = const_cast<UiStats*>(stats.ptr); st->rects_drawn += (u32)items.size();
}

/* Interaction */
struct _UiPointerState { Vector2 pos; bool down; bool pressed; bool released; };
static _UiPointerState _ui_pointer_state() { return { GetMousePosition(), IsMouseButtonDown(MOUSE_BUTTON_LEFT), IsMouseButtonPressed(MOUSE_BUTTON_LEFT), IsMouseButtonReleased(MOUSE_BUTTON_LEFT) }; }
static void _ui_process_one_interaction(const _UiPointerState &ptr, UiPosition *pos, UiRectSize *size, UiInteraction *interaction) {
    const Rectangle rect = {pos->pos.x, pos->pos.y, size->size.x, size->size.y};
    const bool inside = ptr.pos.x >= rect.x && ptr.pos.x <= rect.x + rect.width && ptr.pos.y >= rect.y && ptr.pos.y <= rect.y + rect.height;
    UiInteractionState new_state = UiInteractionState::None;
    if (inside) new_state = ptr.down ? UiInteractionState::Pressed : UiInteractionState::Hovered;
    interaction->previous = interaction->state; interaction->state = new_state;
}

static void _ui_interaction_system(ecs::Query<ecs::Mut<UiPosition>, ecs::Mut<UiRectSize>, ecs::Optional<UiButton>, ecs::Optional<UiInteraction>, ecs::Optional<UiStyle>> q, ecs::Res<UiStats> stats) {
    UiScopedTimerInternal t(const_cast<double*>(&stats.ptr->interaction_ms));
    const _UiPointerState ptr = _ui_pointer_state();
    u32 count = 0;
    for (auto [pos_w, size_w, button_w, interaction_w, style_w] : q) {
        if (!button_w.ptr || !interaction_w.ptr) {
            continue;
        }
        /* Suppress interaction entirely if style marks widget disabled. */
        if (style_w.ptr && style_w.ptr->disabled) {
            auto *inter = const_cast<UiInteraction*>(interaction_w.ptr);
            inter->previous = inter->state; /* keep previous in sync to avoid generating events */
            inter->state = UiInteractionState::None;
            ++count;
            continue;
        }
        _ui_process_one_interaction(ptr, pos_w.ptr, size_w.ptr, const_cast<UiInteraction*>(interaction_w.ptr));
        ++count;
    }
    auto *st = const_cast<UiStats*>(stats.ptr);
    st->interaction_entities += count;
}

/* Interaction events */
static void _ui_interaction_events_system(ecs::Query<ecs::Ref<UiInteraction>, ecs::Optional<UiText>, ecs::Optional<UiStyle>> q, ecs::Res<UiEvents> events, ecs::Res<UiStats> stats) {
    auto *ev = const_cast<UiEvents*>(events.ptr); u32 before = (u32)ev->current.size();
    for (auto [interaction_w, text_w, style_w] : q) {
        const UiInteraction *in = interaction_w.ptr; if (in->state == in->previous) continue; UiEvent e; e.label = text_w.ptr ? text_w.ptr->value : std::string{};
        /* Skip all events if disabled (we still updated interaction->state to None in interaction system). */
        if (style_w.ptr && style_w.ptr->disabled) {
            continue;
        }
        if (in->previous != UiInteractionState::Hovered && in->state == UiInteractionState::Hovered) { e.type = UiEventType::HoverEnter; ev->current.push_back(e); }
        if (in->previous == UiInteractionState::Hovered && in->state == UiInteractionState::None) { e.type = UiEventType::HoverLeave; ev->current.push_back(e); }
        if (in->state == UiInteractionState::Pressed && in->previous != UiInteractionState::Pressed) { e.type = UiEventType::Pressed; ev->current.push_back(e); }
        if (in->previous == UiInteractionState::Pressed && in->state == UiInteractionState::Hovered) { e.type = UiEventType::Released; ev->current.push_back(e); UiEvent click = e; click.type = UiEventType::Click; ev->current.push_back(click); }
    }
    auto *st = const_cast<UiStats*>(stats.ptr); st->events_emitted += ((u32)ev->current.size() - before);
}

/* Style system */
static unsigned char _ui_lerp_u8(unsigned char from, unsigned char to, f32 spd) { f32 f = (f32)from, t = (f32)to; f32 v = f + (t - f) * std::clamp(spd * 0.016f, 0.f, 1.f); return (unsigned char)std::clamp((int)v, 0, 255); }

static void _ui_style_system(ecs::Query<ecs::Mut<UiColor>, ecs::Mut<UiStyle>, ecs::Optional<UiDirty>, ecs::Optional<UiInteraction>, ecs::Ref<UiOriginalColor>> q, ecs::Res<UiTheme> theme, ecs::Res<UiStats> stats) {
    UiScopedTimerInternal t(const_cast<double*>(&stats.ptr->style_ms));
    for (auto [color_w, style_w, dirty_w, interaction_w, original_w] : q) {
        UiStyle *style = style_w.ptr; UiInteraction *inter = const_cast<UiInteraction*>(interaction_w.ptr); const UiOriginalColor *orig = original_w.ptr; UiDirty *dirty = const_cast<UiDirty*>(dirty_w.ptr);
        bool hovered = inter && (inter->state == UiInteractionState::Hovered || inter->state == UiInteractionState::Pressed);
        bool pressed = inter && inter->state == UiInteractionState::Pressed;
        if (dirty && dirty->style) { style->target_r = style->base_r; style->target_g = style->base_g; style->target_b = style->base_b; style->target_a = style->base_a; dirty->style = false; }
        if (style->disabled) { color_w.ptr->r = (unsigned char)(style->base_r * 0.5f); color_w.ptr->g = (unsigned char)(style->base_g * 0.5f); color_w.ptr->b = (unsigned char)(style->base_b * 0.5f); color_w.ptr->a = (unsigned char)(style->base_a * theme.ptr->disabled_alpha); continue; }
        if (hovered && !pressed) {
            f32 percent = style->hover_dark_percent >= 0.f ? style->hover_dark_percent : theme.ptr->hover_dark_percent; f32 factor = 1.f - std::clamp(percent, 0.f, 0.95f);
            style->target_r = (unsigned char)(orig->r * factor); style->target_g = (unsigned char)(orig->g * factor); style->target_b = (unsigned char)(orig->b * factor);
        } else if (!pressed) { style->target_r = orig->r; style->target_g = orig->g; style->target_b = orig->b; }
        if (pressed && inter && inter->state == UiInteractionState::Pressed && inter->previous != UiInteractionState::Pressed) {
            f32 percent = style->flash_percent >= 0.f ? style->flash_percent : theme.ptr->flash_percent; f32 factor = 1.f + std::clamp(percent, 0.f, 1.5f);
            style->target_r = (unsigned char)std::min(255.f, orig->r * factor); style->target_g = (unsigned char)std::min(255.f, orig->g * factor); style->target_b = (unsigned char)std::min(255.f, orig->b * factor);
        }
        color_w.ptr->r = _ui_lerp_u8(color_w.ptr->r, style->target_r, hovered ? theme.ptr->hover_speed : theme.ptr->restore_speed);
        color_w.ptr->g = _ui_lerp_u8(color_w.ptr->g, style->target_g, hovered ? theme.ptr->hover_speed : theme.ptr->restore_speed);
        color_w.ptr->b = _ui_lerp_u8(color_w.ptr->b, style->target_b, hovered ? theme.ptr->hover_speed : theme.ptr->restore_speed);
        color_w.ptr->a = _ui_lerp_u8(color_w.ptr->a, style->target_a, theme.ptr->restore_speed);
    }
}

/* Button action (quit) */
static void _ui_button_action_system(ecs::Query<ecs::Ref<UiButton>, ecs::Optional<UiOnClickQuit>, ecs::Optional<UiInteraction>, ecs::Optional<UiText>, ecs::Optional<UiStyle>> q, ecs::Res<UiTheme> theme, ecs::Res<UiPendingQuit> pending, ecs::Res<UiEvents> events, ecs::Res<UiStats> stats) {
    const bool released = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    auto *ev = const_cast<UiEvents*>(events.ptr);
    auto *st = const_cast<UiStats*>(stats.ptr);
    for (auto [button_w, quit_w, inter_w, text_w, style_w] : q) {
        if (!quit_w.ptr || !inter_w.ptr) {
            continue;
        }
        if (style_w.ptr && style_w.ptr->disabled) {
            continue; /* disabled widgets never trigger actions */
        }
        const UiInteraction *in = inter_w.ptr;
        bool trigger = (released && (in->state == UiInteractionState::Hovered));
        if (trigger) {
            if (theme.ptr->quit_delay <= 0.f) {
                Application::quit = true;
            } else {
                auto *pq = const_cast<UiPendingQuit*>(pending.ptr);
                pq->active = true;
                pq->timer = theme.ptr->quit_delay;
            }
            UiEvent e; e.type = UiEventType::QuitClick; e.label = text_w.ptr ? text_w.ptr->value : "Quit";
            ev->current.push_back(e);
            ++st->events_emitted;
        }
    }
}

/* Pending quit */
static void _ui_pending_quit_system(ecs::Res<core::FrameTime> ft, ecs::Res<UiPendingQuit> pending) {
    if (!pending.ptr->active) {
        return;
    }
    auto *pq = const_cast<UiPendingQuit*>(pending.ptr);
    pq->timer -= ft.ptr->delta_time;
    if (pq->timer <= 0.f) {
        Application::quit = true;
        pq->active = false;
    }
}

/* Frame begin */
static void _ui_events_frame_begin_system(ecs::Res<UiEvents> events, ecs::Res<UiStats> stats) {
    auto *ev = const_cast<UiEvents*>(events.ptr); auto *st = const_cast<UiStats*>(stats.ptr);
    ev->previous.clear(); ev->previous.swap(ev->current); st->events_emitted = 0; st->rects_drawn = 0; st->texts_drawn = 0; st->interaction_entities = 0; st->style_ms = st->interaction_ms = st->render_rect_ms = st->render_text_ms = 0.0; ++st->frame_index;
}

/* Layout stub */
static void _ui_layout_stub_system(ecs::Query<ecs::Mut<UiPosition>, ecs::Optional<UiNode>> q) { (void)q; }

/* Logger */
static const char * _ui_event_type_str(UiEventType t) {
    switch(t) {
        case UiEventType::HoverEnter: return "HoverEnter";
        case UiEventType::HoverLeave: return "HoverLeave";
        case UiEventType::Pressed: return "Pressed";
        case UiEventType::Released: return "Released";
        case UiEventType::Click: return "Click";
        case UiEventType::QuitClick: return "QuitClick";
        case UiEventType::ValueChanged: return "ValueChanged";
        default: return "Unknown";
    }
}

static void _ui_event_logger_system(ecs::Res<UiEvents> events, ecs::Res<UiEventLoggerConfig> cfg) {
    if (!cfg.ptr->enabled) {
        return;
    }
    for (auto &e : events.ptr->current) {
        bool show = false;
        switch(e.type) {
            case UiEventType::HoverEnter:
            case UiEventType::HoverLeave: show = cfg.ptr->log_hover; break;
            case UiEventType::Pressed: show = cfg.ptr->log_press; break;
            case UiEventType::Released: show = cfg.ptr->log_release; break;
            case UiEventType::Click: show = cfg.ptr->log_click; break;
            case UiEventType::QuitClick: show = cfg.ptr->log_quit; break;
            case UiEventType::ValueChanged: show = true; break;
            default: break;
        }
        if (show) {
            TraceLog(LOG_INFO, "[UI][Event] %s label='%s'", _ui_event_type_str(e.type), e.label.c_str());
        }
    }
}

void UiPlugin::build(Application &app) {
    app
        .insert_resource(UiTheme{})
        .insert_resource(UiPendingQuit{})
        .insert_resource(UiEvents{})
        .insert_resource(UiEventLoggerConfig{})
        .insert_resource(UiStats{})
        .add_systems(Schedule::UPDATE,
            _ui_events_frame_begin_system,
            _ui_layout_stub_system,
            _ui_interaction_system,
            _ui_interaction_events_system,
            _ui_style_system,
            _ui_button_action_system,
            _ui_pending_quit_system,
            _ui_event_logger_system)
        .add_systems(Schedule::RENDER, _ui_render_rect_system, _ui_render_text_system);
}

} /* namespace r */
