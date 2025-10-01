#include <R-Engine/Plugins/UIPlugin.hpp>
#include <R-Engine/Application.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/Core/FrameTime.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <fstream>
#include <cstring>

namespace r {

struct UiScopedTimerInternal {
    std::chrono::high_resolution_clock::time_point start;
    double *dst;
    explicit UiScopedTimerInternal(double *d) : start(std::chrono::high_resolution_clock::now()), dst(d) {}
    ~UiScopedTimerInternal() { if (dst) *dst = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - start).count(); }
};

struct UiRenderableTextInternal { UiPosition *pos; UiText *text; UiTextColor *color; UiZIndex *z; UiRectSize *rect_size; UiScale *scale; };
struct UiRenderableRectInternal { UiPosition *pos; UiRectSize *size; UiColor *color; UiBorderColor *border_color; UiBorderThickness *border_thickness; UiBorderRadius *radius; UiZIndex *z; UiScale *scale; };

template <typename T>
/** @brief Helper to sort render batches by z-index ascending. */
static void _ui_sort_by_z(std::vector<T> &items) {
    std::sort(items.begin(), items.end(), [](const T &a, const T &b){
        const i32 za = a.z ? a.z->value : 0; const i32 zb = b.z ? b.z->value : 0; return za < zb; });
}

static void _ui_collect_text_items(ecs::Query<ecs::Mut<UiText>, ecs::Mut<UiPosition>, ecs::Ref<UiTextColor>, ecs::Optional<UiZIndex>, ecs::Optional<UiRectSize>, ecs::Optional<UiScale>> &q, std::vector<UiRenderableTextInternal> &out) {
    for (auto [text_w, pos_w, col_w, z_w, rect_w, scale_w] : q) out.push_back({pos_w.ptr, text_w.ptr, const_cast<UiTextColor *>(col_w.ptr), const_cast<UiZIndex *>(z_w.ptr), const_cast<UiRectSize *>(rect_w.ptr), const_cast<UiScale *>(scale_w.ptr)});
}

static void _ui_collect_rect_items(ecs::Query<ecs::Mut<UiRectSize>, ecs::Mut<UiPosition>, ecs::Ref<UiColor>, ecs::Optional<UiBorderColor>, ecs::Optional<UiBorderThickness>, ecs::Optional<UiBorderRadius>, ecs::Optional<UiZIndex>, ecs::Optional<UiScale>> &q, std::vector<UiRenderableRectInternal> &out) {
    for (auto [size_w, pos_w, col_w, bcol_w, bthick_w, radius_w, z_w, scale_w] : q) out.push_back({pos_w.ptr, size_w.ptr, const_cast<UiColor *>(col_w.ptr), const_cast<UiBorderColor *>(bcol_w.ptr), const_cast<UiBorderThickness *>(bthick_w.ptr), const_cast<UiBorderRadius *>(radius_w.ptr), const_cast<UiZIndex *>(z_w.ptr), const_cast<UiScale *>(scale_w.ptr)});
}

static void _ui_draw_text_items(const std::vector<UiRenderableTextInternal> &items) {
    for (auto &it : items) {
        const float scale = it.scale ? it.scale->value : 1.f;
        const Color c = {it.color->r, it.color->g, it.color->b, it.color->a};
        float draw_x = it.pos->pos.x, draw_y = it.pos->pos.y;
        int font_size = (int)((float)it.text->size * scale);
        if (font_size < 1) font_size = 1;
        const int wpx = MeasureText(it.text->value.c_str(), font_size);
        if (it.rect_size) {
            draw_x += (it.rect_size->size.x - (float)wpx) * 0.5f;
            draw_y += (it.rect_size->size.y - (float)font_size) * 0.5f;
        }
        DrawText(it.text->value.c_str(), (int)draw_x, (int)draw_y, font_size, c);
    }
}

static void _ui_draw_rect_items(const std::vector<UiRenderableRectInternal> &items) {
    for (auto &it : items) {
        const float sc = it.scale ? it.scale->value : 1.f;
        const float w = it.size->size.x * sc;
        const float h = it.size->size.y * sc;
        const float cx = it.pos->pos.x + it.size->size.x * 0.5f;
        const float cy = it.pos->pos.y + it.size->size.y * 0.5f;
        const Rectangle rect = {cx - w * 0.5f, cy - h * 0.5f, w, h};
        const Color c = {it.color->r, it.color->g, it.color->b, it.color->a};
        const f32 radius_value = it.radius ? it.radius->value * sc : 0.f;
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

static void _ui_render_text_system(ecs::Query<ecs::Mut<UiText>, ecs::Mut<UiPosition>, ecs::Ref<UiTextColor>, ecs::Optional<UiZIndex>, ecs::Optional<UiRectSize>, ecs::Optional<UiScale>> q, ecs::Res<UiStats> stats, ecs::Query<ecs::Ref<UiClipRect>> clip_q) {
    UiScopedTimerInternal t(const_cast<double*>(&stats.ptr->render_text_ms));
    std::vector<UiRenderableTextInternal> items; items.reserve(64); _ui_collect_text_items(q, items); _ui_sort_by_z(items);
    for (auto [clip_ref] : clip_q) { (void)clip_ref; }
    _ui_draw_text_items(items);
    auto *st = const_cast<UiStats*>(stats.ptr); st->texts_drawn += (u32)items.size();
}
static void _ui_render_rect_system(ecs::Query<ecs::Mut<UiRectSize>, ecs::Mut<UiPosition>, ecs::Ref<UiColor>, ecs::Optional<UiBorderColor>, ecs::Optional<UiBorderThickness>, ecs::Optional<UiBorderRadius>, ecs::Optional<UiZIndex>, ecs::Optional<UiScale>> q, ecs::Res<UiStats> stats, ecs::Query<ecs::Ref<UiClipRect>> clip_q) {
    UiScopedTimerInternal t(const_cast<double*>(&stats.ptr->render_rect_ms));
    std::vector<UiRenderableRectInternal> items; items.reserve(64); _ui_collect_rect_items(q, items); _ui_sort_by_z(items);
    for (auto [clip_ref] : clip_q) { (void)clip_ref; }
    _ui_draw_rect_items(items);
    auto *st = const_cast<UiStats*>(stats.ptr); st->rects_drawn += (u32)items.size();
}

struct _UiPointerState { Vector2 pos; bool down; bool pressed; bool released; };
static void _ui_process_one_interaction(const _UiPointerState &ptr, UiPosition *pos, UiRectSize *size, UiInteraction *interaction) {
    const Rectangle rect = {pos->pos.x, pos->pos.y, size->size.x, size->size.y};
    const bool inside = ptr.pos.x >= rect.x && ptr.pos.x <= rect.x + rect.width && ptr.pos.y >= rect.y && ptr.pos.y <= rect.y + rect.height;
    UiInteractionState new_state = UiInteractionState::None;
    if (inside) new_state = ptr.down ? UiInteractionState::Pressed : UiInteractionState::Hovered;
    interaction->previous = interaction->state; interaction->state = new_state;
}

static void _ui_input_unified_system(ecs::Res<core::FrameTime> ft, ecs::Res<UiInputState> input, ecs::Res<UiInputConfig> cfg, ecs::Res<UiStats> stats) {
    UiScopedTimerInternal t(const_cast<double*>(&stats.ptr->input_ms));
    auto *in = const_cast<UiInputState*>(input.ptr);
    in->prev_pointer_pos = in->pointer_pos;
    Vector2 mp = GetMousePosition();
    in->pointer_pos = { mp.x, mp.y };
    in->pointer_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    in->pointer_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    in->pointer_released = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    in->any_gamepad = false;
    if (IsGamepadAvailable(cfg.ptr->gamepad_index)) {
        float lx = GetGamepadAxisMovement(cfg.ptr->gamepad_index, GAMEPAD_AXIS_LEFT_X);
        float ly = GetGamepadAxisMovement(cfg.ptr->gamepad_index, GAMEPAD_AXIS_LEFT_Y);
        if (std::fabs(lx) > 0.1f || std::fabs(ly) > 0.1f) {
            in->pointer_pos.x += lx * cfg.ptr->gamepad_cursor_speed * ft.ptr->delta_time;
            in->pointer_pos.y += ly * cfg.ptr->gamepad_cursor_speed * ft.ptr->delta_time;
            in->any_gamepad = true;
        }
        if (IsGamepadButtonPressed(cfg.ptr->gamepad_index, cfg.ptr->confirm_button)) { in->pointer_pressed = true; in->pointer_down = true; }
        if (IsGamepadButtonReleased(cfg.ptr->gamepad_index, cfg.ptr->confirm_button)) { in->pointer_released = true; in->pointer_down = false; }
    }
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    in->pointer_pos.x = std::clamp(in->pointer_pos.x, 0.f, (float)sw - 1.f);
    in->pointer_pos.y = std::clamp(in->pointer_pos.y, 0.f, (float)sh - 1.f);
}

static void _ui_interaction_system(ecs::Query<ecs::Mut<UiPosition>, ecs::Mut<UiRectSize>, ecs::Optional<UiButton>, ecs::Optional<UiInteraction>, ecs::Optional<UiStyle>> q, ecs::Res<UiStats> stats, ecs::Res<UiInputState> input) {
    UiScopedTimerInternal t(const_cast<double*>(&stats.ptr->interaction_ms));
    _UiPointerState ptr{ { input.ptr->pointer_pos.x, input.ptr->pointer_pos.y }, input.ptr->pointer_down, input.ptr->pointer_pressed, input.ptr->pointer_released };
    u32 count = 0;
    for (auto [pos_w, size_w, button_w, interaction_w, style_w] : q) {
        if (!button_w.ptr || !interaction_w.ptr) {
            continue;
        }
        if (style_w.ptr && style_w.ptr->disabled) {
            auto *inter = const_cast<UiInteraction*>(interaction_w.ptr);
            inter->previous = inter->state;
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

static void _ui_interaction_events_system(ecs::Query<ecs::Ref<UiInteraction>, ecs::Optional<UiText>, ecs::Optional<UiStyle>> q, ecs::Res<UiEvents> events, ecs::Res<UiStats> stats) {
    auto *ev = const_cast<UiEvents*>(events.ptr); u32 before = (u32)ev->current.size();
    for (auto [interaction_w, text_w, style_w] : q) {
        const UiInteraction *in = interaction_w.ptr; if (in->state == in->previous) continue; UiEvent e; e.label = text_w.ptr ? text_w.ptr->value : std::string{};
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

/* Pointer move granular event */
static void _ui_pointer_move_event_system(ecs::Res<UiInputState> input, ecs::Res<UiEvents> events, ecs::Res<UiAdvancedLoggerConfig> adv) {
    if (input.ptr->pointer_pos.x != input.ptr->prev_pointer_pos.x || input.ptr->pointer_pos.y != input.ptr->prev_pointer_pos.y) {
        UiEvent e; e.type = UiEventType::PointerMove; e.label = ""; const_cast<UiEvents*>(events.ptr)->current.push_back(e);
    }
    (void)adv;
}

/* Drag detection (per draggable). */
static void _ui_drag_system(ecs::Query<ecs::Mut<UiDragState>, ecs::Ref<UiPosition>, ecs::Ref<UiRectSize>, ecs::Optional<UiDraggable>> q, ecs::Res<UiInputState> input, ecs::Res<UiEvents> events) {
    for (auto [drag_w, pos_w, size_w, drag_marker] : q) {
        if (!drag_marker.ptr) continue;
        auto *ds = drag_w.ptr;
        bool inside = input.ptr->pointer_pos.x >= pos_w.ptr->pos.x && input.ptr->pointer_pos.x <= pos_w.ptr->pos.x + size_w.ptr->size.x && input.ptr->pointer_pos.y >= pos_w.ptr->pos.y && input.ptr->pointer_pos.y <= pos_w.ptr->pos.y + size_w.ptr->size.y;
        if (!ds->dragging && input.ptr->pointer_pressed && inside) {
            ds->dragging = true; ds->start_pos = input.ptr->pointer_pos; UiEvent e; e.type = UiEventType::DragStart; e.label = ""; const_cast<UiEvents*>(events.ptr)->current.push_back(e);
        }
        if (ds->dragging && input.ptr->pointer_released) {
            ds->dragging = false; UiEvent e; e.type = UiEventType::DragEnd; e.label = ""; const_cast<UiEvents*>(events.ptr)->current.push_back(e);
        }
    }
}

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

static void _ui_button_action_system(ecs::Query<ecs::Ref<UiButton>, ecs::Optional<UiOnClickQuit>, ecs::Optional<UiInteraction>, ecs::Optional<UiText>, ecs::Optional<UiStyle>> q, ecs::Res<UiTheme> theme, ecs::Res<UiPendingQuit> pending, ecs::Res<UiEvents> events, ecs::Res<UiStats> stats) {
    const bool released = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    auto *ev = const_cast<UiEvents*>(events.ptr);
    auto *st = const_cast<UiStats*>(stats.ptr);
    for (auto [button_w, quit_w, inter_w, text_w, style_w] : q) {
        if (!quit_w.ptr || !inter_w.ptr) {
            continue;
        }
        if (style_w.ptr && style_w.ptr->disabled) {
            continue;
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

static void _ui_events_frame_begin_system(ecs::Res<UiEvents> events, ecs::Res<UiStats> stats) {
    auto *ev = const_cast<UiEvents*>(events.ptr); auto *st = const_cast<UiStats*>(stats.ptr);
    ev->previous.clear(); ev->previous.swap(ev->current); st->events_emitted = 0; st->rects_drawn = 0; st->texts_drawn = 0; st->interaction_entities = 0; st->style_ms = st->interaction_ms = st->render_rect_ms = st->render_text_ms = 0.0; ++st->frame_index;
}

static void _ui_layout_phase2_system(
    ecs::Query<ecs::Mut<UiRectSize>, ecs::Mut<UiPosition>, ecs::Optional<UiText>, ecs::Optional<UiAutoSizeText>, ecs::Optional<UiMinSize>, ecs::Optional<UiMaxSize>, ecs::Optional<UiPreferredSize>, ecs::Optional<UiScale>> q_all,
    ecs::Query<ecs::Ref<UiStackLayout>, ecs::Ref<UiChildren>, ecs::Ref<UiPosition>, ecs::Optional<UiRectSize>> q_stacks,
    ecs::Res<UiFocusContext> focus_ctx)
{
    for (auto [rect_w, pos_w, text_w, autosize_w, min_w, max_w, pref_w, scale_w] : q_all) {
        (void)pos_w; (void)scale_w;
        if (!text_w.ptr || !autosize_w.ptr) continue;
        if (rect_w.ptr->size.x <= 0.f || rect_w.ptr->size.y <= 0.f) {
            int font_size = (int)text_w.ptr->size;
            int wpx = MeasureText(text_w.ptr->value.c_str(), font_size);
            rect_w.ptr->size.x = (f32)wpx + 12.f;
            rect_w.ptr->size.y = (f32)font_size + 8.f;
            if (pref_w.ptr) const_cast<UiPreferredSize*>(pref_w.ptr)->value = rect_w.ptr->size;
        }
        if (min_w.ptr) {
            rect_w.ptr->size.x = std::max(rect_w.ptr->size.x, min_w.ptr->value.x);
            rect_w.ptr->size.y = std::max(rect_w.ptr->size.y, min_w.ptr->value.y);
        }
        if (max_w.ptr) {
            rect_w.ptr->size.x = std::min(rect_w.ptr->size.x, max_w.ptr->value.x);
            rect_w.ptr->size.y = std::min(rect_w.ptr->size.y, max_w.ptr->value.y);
        }
    }
    for (auto [layout_ref, children_ref, pos_ref, rect_opt] : q_stacks) {
        (void)layout_ref; (void)children_ref; (void)pos_ref; (void)rect_opt;
    }
    auto *fc = const_cast<UiFocusContext*>(focus_ctx.ptr);
    fc->order.clear();
}

static void _ui_focus_navigation_system(ecs::Res<UiFocusContext> focus_ctx, ecs::Res<UiEvents> events) {
    if (!IsKeyPressed(KEY_TAB)) return;
    auto *fc = const_cast<UiFocusContext*>(focus_ctx.ptr);
    if (fc->order.empty()) return;
    size_t idx = 0; bool found = false;
    for (size_t i = 0; i < fc->order.size(); ++i) if (fc->order[i] == fc->current) { idx = i; found = true; break; }
    if (!found) idx = (size_t)-1;
    bool backward = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    size_t next = 0;
    if (backward) {
        if (idx == (size_t)-1) next = fc->order.size() - 1; else next = (idx == 0 ? fc->order.size() - 1 : idx - 1);
    } else {
        if (idx == (size_t)-1) next = 0; else next = (idx + 1) % fc->order.size();
    }
    ecs::Entity new_focus = fc->order[next];
    if (new_focus != fc->current) {
        UiEvent leave; leave.type = UiEventType::FocusLeave; UiEvent enter; enter.type = UiEventType::FocusEnter;
        auto *ev = const_cast<UiEvents*>(events.ptr);
        if (fc->current != 0) ev->current.push_back(leave);
        ev->current.push_back(enter);
        fc->previous = fc->current; fc->current = new_focus;
    }
}

static void _ui_name_registry_system(ecs::Query<ecs::Ref<UiName>> q, ecs::Res<UiNameRegistry> reg) {
    auto *r = const_cast<UiNameRegistry*>(reg.ptr);
    r->map.clear();
    for (auto [name_w] : q) {
        (void)name_w;
    }
}
static void _ui_style_inheritance_system(ecs::Query<ecs::Mut<UiOriginalColor>, ecs::Optional<UiStyleInherit>, ecs::Optional<UiInheritedBaseColor>> q) {
    for (auto [orig_w, inherit_w, inherited_w] : q) {
        if (!inherit_w.ptr || !inherited_w.ptr) continue;
        orig_w.ptr->r = inherited_w.ptr->r;
        orig_w.ptr->g = inherited_w.ptr->g;
        orig_w.ptr->b = inherited_w.ptr->b;
        orig_w.ptr->a = inherited_w.ptr->a;
    }
}

static void _ui_toggle_system(ecs::Query<ecs::Mut<UiToggle>, ecs::Ref<UiInteraction>> q, ecs::Res<UiEvents> events) {
    auto *ev = const_cast<UiEvents*>(events.ptr);
    for (auto [toggle_w, interaction_w] : q) {
        const UiInteraction *in = interaction_w.ptr;
        if (in->previous == UiInteractionState::Pressed && in->state == UiInteractionState::Hovered) {
            toggle_w.ptr->value = !toggle_w.ptr->value;
            UiEvent e; e.type = UiEventType::ValueChanged; e.label = toggle_w.ptr->value ? "true" : "false"; ev->current.push_back(e);
        }
    }
}

static void _ui_slider_system(ecs::Query<ecs::Mut<UiSlider>, ecs::Ref<UiPosition>, ecs::Ref<UiRectSize>, ecs::Optional<UiInteraction>> q, ecs::Res<UiEvents> events, ecs::Res<UiInputState> input) {
    auto *ev = const_cast<UiEvents*>(events.ptr);
    for (auto [slider_w, pos_w, size_w, inter_w] : q) {
        auto *sl = slider_w.ptr;
        if (!inter_w.ptr) continue;
        bool inside = input.ptr->pointer_pos.x >= pos_w.ptr->pos.x && input.ptr->pointer_pos.x <= pos_w.ptr->pos.x + size_w.ptr->size.x && input.ptr->pointer_pos.y >= pos_w.ptr->pos.y && input.ptr->pointer_pos.y <= pos_w.ptr->pos.y + size_w.ptr->size.y;
        if (input.ptr->pointer_pressed && inside) sl->dragging = true;
        if (input.ptr->pointer_released) sl->dragging = false;
        if (sl->dragging) {
            float rel = (input.ptr->pointer_pos.x - pos_w.ptr->pos.x) / std::max(1.f, size_w.ptr->size.x);
            rel = std::clamp(rel, 0.f, 1.f);
            float new_val = sl->min + (sl->max - sl->min) * rel;
            if (std::fabs(new_val - sl->value) > 0.0001f) {
                sl->value = new_val;
                UiEvent e; e.type = UiEventType::ValueChanged; e.label = "slider"; ev->current.push_back(e);
            }
        }
    }
}

static void _ui_label_bind_slider_system(ecs::Query<ecs::Mut<UiText>, ecs::Ref<UiLabelBindSlider>> q, ecs::Res<UiSliderValueCache> cache) {
    auto *c = const_cast<UiSliderValueCache*>(cache.ptr);
    for (auto [text_w, bind_w] : q) {
        for (auto &p : c->values) if (p.first == bind_w.ptr->slider) {
            std::ostringstream ss; ss.setf(std::ios::fixed); ss.precision(2); ss << p.second; text_w.ptr->value = ss.str();
            break;
        }
    }
}

static void _ui_virtual_list_system(ecs::Query<ecs::Mut<UiVirtualList>, ecs::Ref<UiRectSize>> q) {
    for (auto [list_w, rect_w] : q) {
        auto *vl = list_w.ptr;
        vl->visible_count = (i32)std::clamp((int)(rect_w.ptr->size.y / vl->item_height), 0, (int)vl->items.size());
        if (vl->first_visible + vl->visible_count > (i32)vl->items.size()) vl->first_visible = std::max(0, (int)vl->items.size() - vl->visible_count);
    }
}

static const char * _ui_event_type_str(UiEventType t) {
    switch(t) {
        case UiEventType::HoverEnter: return "HoverEnter";
        case UiEventType::HoverLeave: return "HoverLeave";
        case UiEventType::Pressed: return "Pressed";
        case UiEventType::Released: return "Released";
        case UiEventType::Click: return "Click";
        case UiEventType::QuitClick: return "QuitClick";
        case UiEventType::ValueChanged: return "ValueChanged";
        case UiEventType::FocusEnter: return "FocusEnter";
        case UiEventType::FocusLeave: return "FocusLeave";
        default: return "Unknown";
    }
}

static bool _ui_should_throttle(double now_ms, double *last_ms, u32 throttle_ms) {
    if (!last_ms) return true;
    if (throttle_ms == 0) return true;
    if (*last_ms < 0.0 || (now_ms - *last_ms) >= (double)throttle_ms) { *last_ms = now_ms; return true; }
    return false;
}

static void _ui_event_logger_system(ecs::Res<UiEvents> events, ecs::Res<UiEventLoggerConfig> cfg, ecs::Res<UiAdvancedLoggerConfig> adv) {
    if (!cfg.ptr->enabled && !adv.ptr->enabled) {
        return;
    }
    double now_ms = GetTime() * 1000.0;
    auto *adv_mut = const_cast<UiAdvancedLoggerConfig*>(adv.ptr);
    for (auto &e : events.ptr->current) {
        bool show = false;
        bool throttle_ok = true;
        switch(e.type) {
            case UiEventType::HoverEnter:
            case UiEventType::HoverLeave: show = cfg.ptr->log_hover; break;
            case UiEventType::Pressed: show = cfg.ptr->log_press; break;
            case UiEventType::Released: show = cfg.ptr->log_release; break;
            case UiEventType::Click: show = cfg.ptr->log_click; break;
            case UiEventType::QuitClick: show = cfg.ptr->log_quit; break;
            case UiEventType::ValueChanged: show = true; break;
            case UiEventType::FocusEnter:
            case UiEventType::FocusLeave: show = true; break;
            default: break;
        }
        if (adv_mut->enabled) {
            switch(e.type) {
                case UiEventType::HoverEnter:
                case UiEventType::HoverLeave: throttle_ok = _ui_should_throttle(now_ms, &adv_mut->last_hover_time, adv_mut->throttle_hover_ms); break;
                case UiEventType::Pressed: throttle_ok = _ui_should_throttle(now_ms, &adv_mut->last_press_time, adv_mut->throttle_press_ms); break;
                case UiEventType::Released: throttle_ok = _ui_should_throttle(now_ms, &adv_mut->last_release_time, adv_mut->throttle_release_ms); break;
                case UiEventType::Click: throttle_ok = _ui_should_throttle(now_ms, &adv_mut->last_click_time, adv_mut->throttle_click_ms); break;
                case UiEventType::FocusEnter:
                case UiEventType::FocusLeave: throttle_ok = _ui_should_throttle(now_ms, &adv_mut->last_focus_time, adv_mut->throttle_focus_ms); break;
                case UiEventType::PointerMove: throttle_ok = _ui_should_throttle(now_ms, &adv_mut->last_pointer_time, adv_mut->throttle_pointer_ms); break;
                case UiEventType::DragStart:
                case UiEventType::DragEnd: throttle_ok = _ui_should_throttle(now_ms, &adv_mut->last_drag_time, adv_mut->throttle_drag_ms); break;
                default: break;
            }
        }
        if ((cfg.ptr->enabled || adv_mut->enabled) && show && throttle_ok) {
            TraceLog(LOG_INFO, "[UI][Event] %s label='%s'", _ui_event_type_str(e.type), e.label.c_str());
        }
    }
}

static void _ui_animation_system(ecs::Query<ecs::Mut<UiScale>, ecs::Optional<UiAnimations>> q, ecs::Res<core::FrameTime> ft, ecs::Res<UiStats> stats) {
    UiScopedTimerInternal t(const_cast<double*>(&stats.ptr->animation_ms));
    for (auto [scale_w, anims_w] : q) {
        if (!anims_w.ptr) continue;
        auto *sc = scale_w.ptr;
        for (auto &track : const_cast<UiAnimations*>(anims_w.ptr)->tracks) {
            if (!track.playing) continue;
            track.elapsed += (f32)ft.ptr->delta_time;
            float alpha = track.duration > 0.f ? std::clamp(track.elapsed / track.duration, 0.f, 1.f) : 1.f;
            float v = track.from + (track.to - track.from) * alpha;
            if (track.property == UiAnimProperty::Scale) {
                sc->target = v;
            }
            if (alpha >= 1.f) {
                if (track.loop) { track.elapsed = 0.f; std::swap(track.from, track.to); }
                else track.playing = false;
            }
        }
    }
}

/* Timeline animation system */
static void _ui_timeline_system(ecs::Query<ecs::Mut<UiScale>, ecs::Optional<UiTimeline>> q, ecs::Res<core::FrameTime> ft) {
    for (auto [scale_w, timeline_w] : q) {
        if (!timeline_w.ptr) continue;
        auto *tl = const_cast<UiTimeline*>(timeline_w.ptr);
        if (!tl->playing) continue;
        tl->time += (float)ft.ptr->delta_time;
        for (auto &track : tl->tracks) {
            if (track.keys.empty()) continue;
            float total = track.keys.back().time;
            float t = tl->time;
            if (track.loop && total > 0.f) t = fmodf(t, total); else if (t > total) t = total;
            UiTimelineKeyframe a = track.keys.front();
            UiTimelineKeyframe b = track.keys.back();
            for (size_t i = 1; i < track.keys.size(); ++i) {
                if (t <= track.keys[i].time) { a = track.keys[i-1]; b = track.keys[i]; break; }
            }
            float alpha = (b.time - a.time) > 0.f ? (t - a.time) / (b.time - a.time) : 1.f;
            alpha = std::clamp(alpha, 0.f, 1.f);
            float v = a.value + (b.value - a.value) * alpha;
            if (track.property == UiTimelineProperty::Scale)
                scale_w.ptr->target = v;
        }
    }
}

/* Headless scripted input override */
static void _ui_headless_input_system(ecs::Res<UiHeadlessConfig> cfg, ecs::Res<UiHeadlessScript> script, ecs::Res<UiInputState> input) {
    if (!cfg.ptr->enabled) return;
    auto *s = const_cast<UiHeadlessScript*>(script.ptr);
    if (s->index >= s->events.size()) return;
    auto ev = s->events[s->index++];
    auto *in = const_cast<UiInputState*>(input.ptr);
    in->prev_pointer_pos = in->pointer_pos;
    in->pointer_pos = ev.pointer_pos;
    in->pointer_pressed = ev.press;
    in->pointer_released = ev.release;
    if (ev.press) in->pointer_down = true;
    if (ev.release) in->pointer_down = false;
}

/* Theme file loader */
bool load_ui_theme_file(Application &app, const std::string &path) {
    std::ifstream f(path.c_str());
    if (!f.is_open()) return false;
    UiTheme theme;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string k = line.substr(0, eq);
        std::string v = line.substr(eq + 1);
        auto to_f = [&](const std::string &s){ return (float)atof(s.c_str()); };
        if (k == "hover_dark_percent") theme.hover_dark_percent = to_f(v);
        else if (k == "hover_speed") theme.hover_speed = to_f(v);
        else if (k == "restore_speed") theme.restore_speed = to_f(v);
        else if (k == "flash_percent") theme.flash_percent = to_f(v);
        else if (k == "flash_speed") theme.flash_speed = to_f(v);
        else if (k == "quit_delay") theme.quit_delay = to_f(v);
        else if (k == "press_flash_percent") theme.press_flash_percent = to_f(v);
        else if (k == "disabled_alpha") theme.disabled_alpha = to_f(v);
    }
    app.insert_resource(theme);
    return true;
}

void UiPlugin::build(Application &app) {
    app
        .insert_resource(UiTheme{})
        .insert_resource(UiPendingQuit{})
        .insert_resource(UiEvents{})
        .insert_resource(UiEventLoggerConfig{})
        .insert_resource(UiAdvancedLoggerConfig{})
        .insert_resource(UiStats{})
        .insert_resource(UiInputState{})
        .insert_resource(UiInputConfig{})
        .insert_resource(UiTextMeasureCache{})
        .insert_resource(UiProfiler{})
        .insert_resource(UiHeadlessConfig{})
        .insert_resource(UiHeadlessScript{})
        .add_systems(Schedule::UPDATE,
            _ui_events_frame_begin_system,
            _ui_headless_input_system,
            _ui_layout_phase2_system,
            _ui_input_unified_system,
            _ui_pointer_move_event_system,
            _ui_interaction_system,
            _ui_interaction_events_system,
            _ui_style_system,
            _ui_animation_system,
            _ui_timeline_system,
            _ui_focus_navigation_system,
            _ui_style_inheritance_system,
            _ui_toggle_system,
            _ui_slider_system,
            _ui_label_bind_slider_system,
            _ui_virtual_list_system,
            _ui_name_registry_system,
            _ui_drag_system,
            _ui_button_action_system,
            _ui_pending_quit_system,
            _ui_event_logger_system)
        .add_systems(Schedule::RENDER, _ui_render_rect_system, _ui_render_text_system)
        .insert_resource(UiFocusContext{})
        .insert_resource(UiNameRegistry{})
        .insert_resource(UiSliderValueCache{});
}

}
