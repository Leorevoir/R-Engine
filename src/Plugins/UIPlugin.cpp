#include <R-Engine/Plugins/UIPlugin.hpp>

#include <R-Engine/Application.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/Core/FrameTime.hpp>

#include <raylib.h>
#include <algorithm>

using namespace r;

static bool _ui_pending_quit_active = false;
static float _ui_pending_quit_timer = 0.f;

struct _UiRenderableText { ecs::Entity e; UiPosition *pos; UiText *text; UiTextColor *color; UiZIndex *z; UiRectSize *rect_size; };
struct _UiRenderableRect { ecs::Entity e; UiPosition *pos; UiRectSize *size; UiColor *color; UiBorderColor *border_color; UiBorderThickness *border_thickness; UiBorderRadius *radius; UiZIndex *z; };

static void _render_ui_text(ecs::Query<ecs::Mut<UiText>, ecs::Mut<UiPosition>, ecs::Ref<UiTextColor>, ecs::Optional<UiZIndex>, ecs::Optional<UiRectSize>> q)
{
    std::vector<_UiRenderableText> items;
    for (auto [text_w, pos_w, col_w, z_w, rect_w] : q) {
        items.push_back({0, pos_w.ptr, text_w.ptr, const_cast<UiTextColor *>(col_w.ptr), const_cast<UiZIndex *>(z_w.ptr), const_cast<UiRectSize *>(rect_w.ptr)});
    }
    std::sort(items.begin(), items.end(), [] (const _UiRenderableText &a, const _UiRenderableText &b) {
        const i32 za = a.z ? a.z->value : 0;
        const i32 zb = b.z ? b.z->value : 0;
        return za < zb;
    });
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

static void _render_ui_rect(ecs::Query<ecs::Mut<UiRectSize>, ecs::Mut<UiPosition>, ecs::Ref<UiColor>, ecs::Optional<UiBorderColor>, ecs::Optional<UiBorderThickness>, ecs::Optional<UiBorderRadius>, ecs::Optional<UiZIndex>> q)
{
    std::vector<_UiRenderableRect> items;
    for (auto [size_w, pos_w, col_w, bcol_w, bthick_w, radius_w, z_w] : q) {
        items.push_back({0, pos_w.ptr, size_w.ptr, const_cast<UiColor *>(col_w.ptr), const_cast<UiBorderColor *>(bcol_w.ptr), const_cast<UiBorderThickness *>(bthick_w.ptr), const_cast<UiBorderRadius *>(radius_w.ptr), const_cast<UiZIndex *>(z_w.ptr)});
    }
    std::sort(items.begin(), items.end(), [] (const _UiRenderableRect &a, const _UiRenderableRect &b) {
        const i32 za = a.z ? a.z->value : 0;
        const i32 zb = b.z ? b.z->value : 0;
        return za < zb;
    });
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

static void _ui_button_interaction(ecs::Query<ecs::Mut<UiPosition>, ecs::Mut<UiRectSize>, ecs::Mut<UiColor>, ecs::Optional<UiButton>, ecs::Optional<UiButtonState>, ecs::Optional<UiOriginalColor>, ecs::Optional<UiOnClickQuit>> q, ecs::Res<UiTheme> theme)
{
    const Vector2 mouse = GetMousePosition();
    const bool mouse_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    const bool mouse_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    const bool mouse_released = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

    for (auto [pos_w, size_w, color_w, button_w, state_w, original_w, quit_w] : q) {
        if (!button_w.ptr) {
            continue;
        }
        UiButtonState *state = nullptr;
        if (!state_w.ptr) {
            continue;
        } else {
            state = const_cast<UiButtonState *>(state_w.ptr);
        }
        UiOriginalColor *orig = nullptr;
        if (original_w.ptr) {
            orig = const_cast<UiOriginalColor *>(original_w.ptr);
        } else {
        }
        const Rectangle rect = {pos_w.ptr->pos.x, pos_w.ptr->pos.y, size_w.ptr->size.x, size_w.ptr->size.y};
        const bool inside = mouse.x >= rect.x && mouse.x <= rect.x + rect.width && mouse.y >= rect.y && mouse.y <= rect.y + rect.height;
        state->hovered = inside;
        if (inside && mouse_pressed) {
            state->pressed = true;
        }
        if (state->pressed && mouse_released) {
            if (inside && quit_w.ptr) {
                if (theme.ptr->quit_delay <= 0.f) {
                    r::Application::quit = true;
                } else {
                    _ui_pending_quit_timer = theme.ptr->quit_delay;
                    _ui_pending_quit_active = true;
                }
            }
            state->pressed = false;
        }
        if (!mouse_down && !inside && !state->hovered) {
            state->pressed = false;
        }

        if (orig) {
            const f32 target_dark_factor = 1.f - std::clamp(theme.ptr->hover_dark_percent, 0.f, 0.95f);
            const f32 flash_factor = 1.f + std::clamp(theme.ptr->flash_percent, 0.f, 1.f);
            const f32 hover_speed = theme.ptr->hover_speed;
            const f32 restore_speed = theme.ptr->restore_speed;
            const f32 flash_speed = theme.ptr->flash_speed;

            auto lerp_u8 = [] (unsigned char from, unsigned char to, f32 spd) {
                f32 f = static_cast<f32>(from);
                f32 t = static_cast<f32>(to);
                f32 v = f + (t - f) * std::clamp(spd * 0.016f, 0.f, 1.f);
                return static_cast<unsigned char>(std::clamp(static_cast<int>(v), 0, 255));
            };

            const int dark_r = static_cast<int>(orig->r * target_dark_factor);
            const int dark_g = static_cast<int>(orig->g * target_dark_factor);
            const int dark_b = static_cast<int>(orig->b * target_dark_factor);
            const int flash_r = static_cast<int>(std::min<f32>(255.f, orig->r * flash_factor));
            const int flash_g = static_cast<int>(std::min<f32>(255.f, orig->g * flash_factor));
            const int flash_b = static_cast<int>(std::min<f32>(255.f, orig->b * flash_factor));

            if (state->hovered && !state->pressed) {
                color_w.ptr->r = lerp_u8(color_w.ptr->r, static_cast<unsigned char>(dark_r), hover_speed);
                color_w.ptr->g = lerp_u8(color_w.ptr->g, static_cast<unsigned char>(dark_g), hover_speed);
                color_w.ptr->b = lerp_u8(color_w.ptr->b, static_cast<unsigned char>(dark_b), hover_speed);
            } else if (!state->hovered && !state->pressed) {
                color_w.ptr->r = lerp_u8(color_w.ptr->r, orig->r, restore_speed);
                color_w.ptr->g = lerp_u8(color_w.ptr->g, orig->g, restore_speed);
                color_w.ptr->b = lerp_u8(color_w.ptr->b, orig->b, restore_speed);
            }

            if (inside && mouse_pressed) {
                color_w.ptr->r = lerp_u8(color_w.ptr->r, static_cast<unsigned char>(flash_r), flash_speed);
                color_w.ptr->g = lerp_u8(color_w.ptr->g, static_cast<unsigned char>(flash_g), flash_speed);
                color_w.ptr->b = lerp_u8(color_w.ptr->b, static_cast<unsigned char>(flash_b), flash_speed);
            }
        }
    }
}

static void _ui_quit_delay_system(ecs::Res<core::FrameTime> ft)
{
    if (_ui_pending_quit_active) {
        _ui_pending_quit_timer -= ft.ptr->delta_time;
        if (_ui_pending_quit_timer <= 0.f) {
            r::Application::quit = true;
            _ui_pending_quit_active = false;
        }
    }
}

void UiPlugin::build(Application &app)
{
    app
        .insert_resource(UiTheme{})
        .add_systems(Schedule::UPDATE, _ui_button_interaction, _ui_quit_delay_system)
        .add_systems(Schedule::RENDER, _render_ui_rect, _render_ui_text);
}
