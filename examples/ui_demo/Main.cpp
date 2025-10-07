#include <R-Engine/Application.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>
#include <R-Engine/Plugins/UiPlugin.hpp>
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/UI/Components.hpp>
#include <R-Engine/UI/Text.hpp>
#include <R-Engine/UI/Image.hpp>
#include <R-Engine/UI/Button.hpp>
#include <R-Engine/UI/Theme.hpp>
#include <R-Engine/Core/Backend.hpp>

/* === Phase 6, 7 & 8 Demo === */

/* Build a simple UI: title + row of themed buttons (text + image) */
static void build_ui(r::ecs::Commands &cmds, r::ecs::Res<r::WindowPluginConfig> win)
{
    const float ww = static_cast<float>(win.ptr->size.width);
    const float wh = static_cast<float>(win.ptr->size.height);

    auto root = cmds.spawn(
        r::UiNode{},
        r::Style{ .width = 0.f, .height = 0.f, .width_pct = 100.f, .height_pct = -1.f, .background = {0,0,0,0}, .z_index = 0 },
        r::ComputedLayout{}, r::Visibility::Visible);
    const r::ecs::Entity root_id = root.id();

    auto column = cmds.spawn(
        r::UiNode{}, r::Parent{ root_id },
        r::Style{ .width = 0.f, .height = 360.f, .width_pct = 90.f, .height_pct = -1.f, .min_width = 0.f, .max_width = 0.f, .min_height = 0.f, .max_height = 0.f, .background = {60, 60, 80, 255}, .z_index = 1, .margin = 0.f, .padding = 0.f, .direction = r::LayoutDirection::Column, .justify = r::JustifyContent::Start, .align = r::AlignItems::Start, .align_self = r::AlignSelf::Auto, .position = r::PositionType::Relative, .offset_x = 0.f, .offset_y = 0.f, .gap = 0.f, .clip_children = false, .overflow_clip = false, .border_thickness = 2.f, .border_color = {180,180,180,255} },
        r::ComputedLayout{}, r::Visibility::Visible);
    const r::ecs::Entity col_id = column.id();

    /* Title uses theme text color and default font when color alpha == 0 */
    cmds.spawn(
        r::UiNode{}, r::Parent{ col_id },
        r::Style{ .width = 880.f, .height = 48.f, .background = {0,0,0,0}, .z_index = 2 },
        r::UiText{ .content = std::string("R-Engine UI — Themed Buttons"), .font_size = 28, .wrap_width = 0.f, .color = {0,0,0,0} },
        r::ComputedLayout{}, r::Visibility::Visible);

    auto row = cmds.spawn(
        r::UiNode{}, r::Parent{ col_id },
        r::Style{ .width = 0.f, .height = 120.f, .width_pct = 100.f, .height_pct = -1.f, .min_width = 0.f, .max_width = 0.f, .min_height = 0.f, .max_height = 0.f, .background = {0,0,0,0}, .z_index = 2, .margin = 0.f, .padding = 0.f, .direction = r::LayoutDirection::Row, .justify = r::JustifyContent::Start, .align = r::AlignItems::Center, .align_self = r::AlignSelf::Auto, .position = r::PositionType::Relative, .offset_x = 0.f, .offset_y = 0.f, .gap = 12.f, .clip_children = true, .overflow_clip = true, .border_thickness = 0.f, .border_color = {0,0,0,255} },
        r::ComputedLayout{}, r::Visibility::Visible);
    const r::ecs::Entity row_id = row.id();

    /* Text button (enabled) */
    cmds.spawn(
        r::UiNode{}, r::Parent{ row_id }, r::UiButton{},
        r::Style{ .width = 0.f, .height = 64.f, .width_pct = 30.f, .height_pct = -1.f, .min_width = 160.f, .max_width = 0.f, .min_height = 0.f, .max_height = 0.f, .background = {0,0,0,0}, .z_index = 5, .margin = 0.f, .padding = 0.f, .direction = r::LayoutDirection::Column, .justify = r::JustifyContent::Center, .align = r::AlignItems::Center, .align_self = r::AlignSelf::Auto, .position = r::PositionType::Relative, .offset_x = 0.f, .offset_y = 0.f, .gap = 0.f, .clip_children = false, .overflow_clip = false, .border_thickness = 0.f, .border_color = {0,0,0,255} },
        r::UiText{ .content = std::string("Play"), .font_size = 24, .wrap_width = 0.f, .color = {0,0,0,0} },
        r::ComputedLayout{}, r::Visibility::Visible);

    /* Image button (disabled) */
    cmds.spawn(
        r::UiNode{}, r::Parent{ row_id }, r::UiButton{ .disabled = true },
        r::Style{ .width = 0.f, .height = 64.f, .width_pct = 30.f, .height_pct = -1.f, .min_width = 160.f, .max_width = 0.f, .min_height = 0.f, .max_height = 0.f, .background = {0,0,0,0}, .z_index = 5, .margin = 0.f, .padding = 0.f, .direction = r::LayoutDirection::Column, .justify = r::JustifyContent::Center, .align = r::AlignItems::Center, .align_self = r::AlignSelf::End, .position = r::PositionType::Relative, .offset_x = 0.f, .offset_y = 0.f, .gap = 0.f, .clip_children = false, .overflow_clip = false, .border_thickness = 0.f, .border_color = {0,0,0,255} },
        r::UiImage{ .path = std::string("examples/ui_demo/icon.png"), .tint = {255,255,255,255}, .keep_aspect = true },
        r::ComputedLayout{}, r::Visibility::Visible);

    /* Absolute badge in top-right of column */
    cmds.spawn(
        r::UiNode{}, r::Parent{ col_id },
        r::Style{ .width = 120.f, .height = 32.f, .width_pct = -1.f, .height_pct = -1.f, .min_width = 0.f, .max_width = 0.f, .min_height = 0.f, .max_height = 0.f, .background = {0,0,0,0}, .z_index = 10, .margin = 0.f, .padding = 0.f, .direction = r::LayoutDirection::Column, .justify = r::JustifyContent::Start, .align = r::AlignItems::Start, .align_self = r::AlignSelf::Auto, .position = r::PositionType::Absolute, .offset_x = 740.f, .offset_y = 8.f, .gap = 0.f, .clip_children = false, .overflow_clip = false, .border_thickness = 0.f, .border_color = {0,0,0,255} },
        r::UiText{ .content = std::string("v0.1 demo"), .font_size = 18, .wrap_width = 0.f, .color = {0,0,0,0} },
        r::ComputedLayout{}, r::Visibility::Visible);

    /* Scrollable list (overflow clip + UiScroll). Use percent width to adapt. */
    auto scroll_area = cmds.spawn(
        r::UiNode{}, r::Parent{ col_id }, r::UiScroll{ .x = 0.f, .y = 0.f },
        r::Style{ .width = 0.f, .height = 160.f, .width_pct = 100.f, .height_pct = -1.f, .min_width = 0.f, .max_width = 0.f, .min_height = 0.f, .max_height = 0.f, .background = {0,0,0,0}, .z_index = 2, .margin = 0.f, .padding = 0.f, .direction = r::LayoutDirection::Column, .justify = r::JustifyContent::Start, .align = r::AlignItems::Start, .align_self = r::AlignSelf::Auto, .position = r::PositionType::Relative, .offset_x = 0.f, .offset_y = 0.f, .gap = 6.f, .clip_children = true, .overflow_clip = true, .border_thickness = 2.f, .border_color = {160,160,160,255} },
        r::ComputedLayout{}, r::Visibility::Visible);
    const r::ecs::Entity scroll_parent = scroll_area.id();
    for (int i = 0; i < 20; ++i) {
        cmds.spawn(
            r::UiNode{}, r::Parent{ scroll_parent },
            r::Style{ .width = 0.f, .height = 40.f, .width_pct = 100.f, .height_pct = -1.f, .min_width = 0.f, .max_width = 0.f, .min_height = 0.f, .max_height = 0.f, .background = { (u8)(80 + (i*7)%100), (u8)(100 + (i*5)%100), (u8)(150 + (i*3)%100), 255}, .z_index = 2, .margin = 0.f, .padding = 0.f, .direction = r::LayoutDirection::Row, .justify = r::JustifyContent::Start, .align = r::AlignItems::Center, .align_self = r::AlignSelf::Auto, .position = r::PositionType::Relative, .offset_x = 0.f, .offset_y = 0.f, .gap = 0.f, .clip_children = false, .overflow_clip = false, .border_thickness = 0.f, .border_color = {0,0,0,255} },
            r::UiText{ .content = std::string("Item ") + std::to_string(i+1), .font_size = 18, .wrap_width = 0.f, .color = {0,0,0,0} },
            r::ComputedLayout{}, r::Visibility::Visible);
    }
}

/* Toggle theme (T): switch spacing/padding and text color/font */
static void theme_toggle_system(r::ecs::Res<r::UserInput> ui, r::ecs::Res<r::InputMap> map, r::ecs::ResMut<r::UiTheme> theme, r::ecs::ResMut<r::UiPluginConfig> cfg)
{
    if (map.ptr->isActionPressed("ToggleTheme", *ui.ptr)) {
        if (theme.ptr->text.r == 230) {
            theme.ptr->text = {30, 30, 30, 255};
            theme.ptr->panel_bg = {230, 230, 235, 255};
            theme.ptr->padding = 10;
            theme.ptr->spacing = 14.f;
            cfg.ptr->overlay_text = "Theme B";
        } else {
            theme.ptr->text = {230, 230, 230, 255};
            theme.ptr->panel_bg = {30, 30, 38, 230};
            theme.ptr->padding = 6;
            theme.ptr->spacing = 8.f;
            cfg.ptr->overlay_text = "Theme A";
        }
    }
}

/* Bind T for theme toggle */
static void setup_theme_controls(r::ecs::ResMut<r::UiPluginConfig> cfg, r::ecs::ResMut<r::InputMap> map)
{
    cfg.ptr->overlay_text = "UI Plugin Ready";
    map.ptr->bindAction("ToggleTheme", r::KEYBOARD, KEY_T);
}

int main()
{
    r::Application{}
        .add_plugins(
            r::DefaultPlugins{}
                .set(r::WindowPlugin{r::WindowPluginConfig{
                    .size = {960, 540},
                    .title = "R-Engine UI Demo (Phase 6 & 7)",
                    .cursor = r::WindowCursorState::Visible,
                }})
        )
        .add_plugins(r::UiPlugin{})
        .add_systems(r::Schedule::STARTUP, setup_theme_controls, build_ui)
        .add_systems(r::Schedule::UPDATE, theme_toggle_system, [](r::ecs::Res<r::UserInput> ui, r::ecs::Query<r::ecs::Mut<r::UiScroll>> q){ float wheel = GetMouseWheelMove(); if (wheel == 0.f) return; for (auto [scroll] : q){ scroll.ptr->y -= wheel * 40.f; if (scroll.ptr->y < 0.f) scroll.ptr->y = 0.f; } })
        .run();
}
