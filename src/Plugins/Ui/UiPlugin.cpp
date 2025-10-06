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

struct Rect { float x; float y; float w; float h; };

static void _layout_recursive(r::ecs::Commands &cmds,
    const std::unordered_map<r::ecs::Entity, std::vector<r::ecs::Entity>> &children,
    const std::unordered_map<r::ecs::Entity, r::Style> &styles,
    r::ecs::Entity node,
    const Rect &content)
{
    auto it = children.find(node);
    if (it == children.end()) {
        return;
    }
    const r::Style parent_style = [&]() {
        auto ps = styles.find(node);
        return (ps != styles.end()) ? ps->second : r::Style{};
    }();

    const auto &kids = it->second;
    const bool row = parent_style.direction == r::LayoutDirection::Row;
    const float main_size = row ? content.w : content.h;

    float total_main = 0.f;
    for (auto child : kids) {
        const r::Style cs = [&]() {
            auto f = styles.find(child);
            return (f != styles.end()) ? f->second : r::Style{};
        }();
        const float mw = (cs.width > 0.f) ? cs.width : 0.f;
        const float mh = (cs.height > 0.f) ? cs.height : 0.f;
        total_main += (row ? mw : mh) + cs.margin * 2.f;
    }

    float free_space = main_size - total_main;
    if (free_space < 0.f) free_space = 0.f;

    float spacing = 0.f;
    float lead = 0.f;
    switch (parent_style.justify) {
        case r::JustifyContent::Start: lead = 0.f; spacing = 0.f; break;
        case r::JustifyContent::Center: lead = free_space * 0.5f; spacing = 0.f; break;
        case r::JustifyContent::End: lead = free_space; spacing = 0.f; break;
        case r::JustifyContent::SpaceBetween:
            spacing = (kids.size() > 1) ? (free_space / static_cast<float>(kids.size() - 1)) : 0.f;
            lead = 0.f;
            break;
        default: lead = 0.f; spacing = 0.f; break;
    }

    float cursor_x = content.x + lead;
    float cursor_y = content.y + lead;

    for (auto child : kids) {
        const r::Style cs = [&]() {
            auto f = styles.find(child);
            return (f != styles.end()) ? f->second : r::Style{};
        }();

        float cw = (cs.width > 0.f) ? cs.width : (row ? 0.f : content.w - cs.margin * 2.f);
        float ch = (cs.height > 0.f) ? cs.height : (row ? content.h - cs.margin * 2.f : 0.f);

        if (parent_style.align == r::AlignItems::Stretch) {
            if (row && cs.height <= 0.f) ch = content.h - cs.margin * 2.f;
            if (!row && cs.width <= 0.f) cw = content.w - cs.margin * 2.f;
        }

        float x = row ? (cursor_x + cs.margin) : (content.x + cs.margin);
        float y = row ? (content.y + cs.margin) : (cursor_y + cs.margin);

        if (row) {
            switch (parent_style.align) {
                case r::AlignItems::Start: y = content.y + cs.margin; break;
                case r::AlignItems::Center: y = content.y + (content.h - ch) * 0.5f; break;
                case r::AlignItems::End: y = content.y + content.h - ch - cs.margin; break;
                case r::AlignItems::Stretch: y = content.y + cs.margin; break;
                default: y = content.y + cs.margin; break;
            }
        } else {
            switch (parent_style.align) {
                case r::AlignItems::Start: x = content.x + cs.margin; break;
                case r::AlignItems::Center: x = content.x + (content.w - cw) * 0.5f; break;
                case r::AlignItems::End: x = content.x + content.w - cw - cs.margin; break;
                case r::AlignItems::Stretch: x = content.x + cs.margin; break;
                default: x = content.x + cs.margin; break;
            }
        }

        cmds.add_component(child, r::ComputedLayout{ x, y, cw, ch, cs.z_index });

        const Rect child_content{ x + cs.padding, y + cs.padding, cw - cs.padding * 2.f, ch - cs.padding * 2.f };
        _layout_recursive(cmds, children, styles, child, child_content);

        if (row) {
            cursor_x += cw + cs.margin * 2.f + spacing;
        } else {
            cursor_y += ch + cs.margin * 2.f + spacing;
        }
    }
}

static void ui_compute_layout_system(
    r::ecs::Commands &cmds,
    r::ecs::Query<
        r::ecs::Optional<r::Parent>,
        r::ecs::Optional<r::Style>,
        r::ecs::Optional<r::Visibility>,
        r::ecs::Ref<r::UiNode>,
        r::ecs::EntityId
    > q,
    r::ecs::Res<r::WindowPluginConfig> win)
{
    std::unordered_map<r::ecs::Entity, std::vector<r::ecs::Entity>> children_map;
    std::unordered_map<r::ecs::Entity, r::Style> styles;
    std::vector<r::ecs::Entity> roots;

    for (auto [parent_opt, style_opt, vis_opt, _node, id] : q) {
        const r::ecs::Entity e = id.value;
        const r::Style s = style_opt.ptr ? *style_opt.ptr : r::Style{};
        styles[e] = s;

        if (!vis_opt.ptr || *vis_opt.ptr == r::Visibility::Visible) {
            if (parent_opt.ptr == nullptr || parent_opt.ptr->id == 0) {
                roots.push_back(e);
            } else {
                children_map[parent_opt.ptr->id].push_back(e);
            }
        }
    }

    const float ww = static_cast<float>(win.ptr->size.width);
    const float wh = static_cast<float>(win.ptr->size.height);

    for (auto root : roots) {
        const r::Style rs = styles[root];
        const float rw = (rs.width > 0.f) ? rs.width : ww;
        const float rh = (rs.height > 0.f) ? rs.height : wh;
        const float rx = 0.f;
        const float ry = 0.f;
        cmds.add_component(root, r::ComputedLayout{ rx, ry, rw, rh, rs.z_index });
        const Rect content{ rx + rs.padding, ry + rs.padding, rw - rs.padding * 2.f, rh - rs.padding * 2.f };
        _layout_recursive(cmds, children_map, styles, root, content);
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
