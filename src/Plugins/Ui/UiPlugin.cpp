#include <R-Engine/Plugins/UiPlugin.hpp>

#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/UI/Fonts.hpp>
#include <R-Engine/UI/InputState.hpp>
#include <R-Engine/UI/Theme.hpp>
#include <R-Engine/UI/Events.hpp>
#include <R-Engine/UI/Components.hpp>
#include <R-Engine/Plugins/RenderPlugin.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>

namespace r {

/**
 * INTERNAL SYSTEMS (skeleton)
 */

static void ui_startup_system(r::ecs::Res<UiPluginConfig> cfg, r::ecs::Res<UiTheme> theme, r::ecs::Res<UiFonts> fonts) noexcept
{
    (void)theme;
    (void)fonts;
    Logger::info(std::string{"UiPlugin startup. DebugOverlay="} + (cfg.ptr->show_debug_overlay ? "on" : "off"));
}

static void ui_update_system(r::ecs::ResMut<UiEvents> events, r::ecs::ResMut<UiInputState> input) noexcept
{
    events.ptr->_reserved.clear();

    const ::Vector2 mp = GetMousePosition();
    input.ptr->mouse_position = {mp.x, mp.y};
    input.ptr->mouse_left_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    input.ptr->mouse_left_released = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    input.ptr->mouse_left_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
}

static constexpr inline ::Camera _to_raylib(const r::Camera3d &c) noexcept
{
    return {
        .position = {c.position.x, c.position.y, c.position.z},
        .target = {c.target.x, c.target.y, c.target.z},
        .up = {c.up.x, c.up.y, c.up.z},
        .fovy = c.fovy,
        .projection = static_cast<int>(c.projection),
    };
}

static void ui_compute_layout_system(
    r::ecs::Query<
        r::ecs::Mut<r::ComputedLayout>,
        r::ecs::Ref<r::UiNode>,
        r::ecs::Optional<r::Parent>,
        r::ecs::Optional<r::Style>
    > q,
    r::ecs::Res<r::WindowPluginConfig> win)
{
    const float root_w = static_cast<float>(win.ptr->size.width);
    const float root_h = static_cast<float>(win.ptr->size.height);

    for (auto [layout, _node, parent_opt, style_opt] : q) {
        const bool is_root = (parent_opt.ptr == nullptr) || (parent_opt.ptr->id == 0);
        const r::Style s = style_opt.ptr ? *style_opt.ptr : r::Style{};

        float x = 0.f, y = 0.f, w = s.width, h = s.height;
        if (is_root) {
            w = (w <= 0.f) ? root_w : w;
            h = (h <= 0.f) ? root_h : h;
            x = 0.f;
            y = 0.f;
        } else {
            const float indent = 20.f;
            x = indent + s.margin + s.padding;
            y = indent + s.margin + s.padding;
            if (w <= 0.f) w = 100.f;
            if (h <= 0.f) h = 60.f;
        }

        layout.ptr->x = x;
        layout.ptr->y = y;
        layout.ptr->w = w;
        layout.ptr->h = h;
        layout.ptr->z = s.z_index;
    }
}

static void ui_render_system(r::ecs::Res<UiPluginConfig> cfg, r::ecs::Res<r::Camera3d> cam,
    r::ecs::Query<r::ecs::Ref<r::UiNode>, r::ecs::Ref<r::ComputedLayout>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::Visibility>> q) noexcept
{
    EndMode3D();

    for (auto [node, layout, style_opt, vis_opt] : q) {
        (void)node;
        const bool hidden = (vis_opt.ptr && (*vis_opt.ptr != r::Visibility::Visible));
        if (hidden) continue;

        const r::Style s = style_opt.ptr ? *style_opt.ptr : r::Style{};
        DrawRectangle((int)layout.ptr->x, (int)layout.ptr->y, (int)layout.ptr->w, (int)layout.ptr->h,
            {s.background.r, s.background.g, s.background.b, s.background.a});
    }
    if (cfg.ptr->show_debug_overlay) {
        DrawRectangle(8, 8, 220, 28, {255, 255, 255, 200});
        const char *text = cfg.ptr->overlay_text.c_str();
        const int font_size = 18;
        DrawText(text, 14, 12, font_size, {0, 0, 0, 255});
        DrawFPS(10, 40);
    }
    BeginMode3D(_to_raylib(*cam.ptr));
}

/**
 * PUBLIC UiPlugin
 */

UiPlugin::UiPlugin(const UiPluginConfig &cfg) noexcept : _config(cfg) {}

void UiPlugin::build(Application &app)
{
    app
        .insert_resource(_config)
        .insert_resource(UiFonts{})
        .insert_resource(UiTheme{})
        .insert_resource(UiInputState{})
        .insert_resource(UiEvents{})
        .add_systems(Schedule::STARTUP, ui_startup_system)
        .add_systems(Schedule::UPDATE, ui_update_system, ui_compute_layout_system)
        .add_systems(Schedule::RENDER, ui_render_system);

    Logger::info("UiPlugin built");
}

}
