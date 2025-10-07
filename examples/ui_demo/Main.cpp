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
#include <R-Engine/UI/Events.hpp>
#include <R-Engine/Core/Backend.hpp>

#include <vector>
#include <string>

/* === Faux menu demo showcasing UiPlugin === */

enum class MenuScreen : u8 { Main, Options, Credits, Levels };

struct MenuState {
    MenuScreen screen = MenuScreen::Main;
    r::ecs::Entity root = 0;

    r::ecs::Entity main_panel = 0;
    r::ecs::Entity options_panel = 0;
    r::ecs::Entity credits_panel = 0;
    r::ecs::Entity levels_panel = 0;

    r::ecs::Entity btn_play = 0;
    r::ecs::Entity btn_options = 0;
    r::ecs::Entity btn_credits = 0;
    r::ecs::Entity btn_quit = 0;

    r::ecs::Entity btn_back_options = 0;
    r::ecs::Entity btn_back_credits = 0;
    r::ecs::Entity btn_back_levels = 0;
    r::ecs::Entity btn_toggle_theme = 0;

    std::vector<r::ecs::Entity> level_items;
    int selected_level = -1;
};

static void set_panel_visibility(r::ecs::Commands &cmds, r::ecs::Entity e, r::Visibility v)
{
    if (e != 0) cmds.add_component(e, v);
}

static void toggle_theme_now(r::ecs::ResMut<r::UiTheme> theme, r::ecs::ResMut<r::UiPluginConfig> cfg)
{
    if (theme.ptr->text.r == 230) {
        theme.ptr->text = {30, 30, 30, 255};
        theme.ptr->panel_bg = {230, 230, 235, 255};
        cfg.ptr->overlay_text = "Theme B";
    } else {
        theme.ptr->text = {230, 230, 230, 255};
        theme.ptr->panel_bg = {30, 30, 38, 230};
        cfg.ptr->overlay_text = "Theme A";
    }
}

/* Build a centered menu with multiple screens */
static void build_menu_ui(r::ecs::Commands &cmds, r::ecs::Res<r::WindowPluginConfig> win, r::ecs::ResMut<MenuState> state)
{
    (void)win;

    auto root = cmds.spawn(
        r::UiNode{},
        r::Style{ .width = 0.f, .height = 0.f, .width_pct = 100.f, .height_pct = 100.f, .background = {0,0,0,0}, .z_index = 0, .padding = 8.f },
        r::ComputedLayout{}, r::Visibility::Visible);
    state.ptr->root = root.id();

    /* Layout grid: top spacer, middle row (left spacer, panel, right spacer), bottom spacer */
    auto vbox = cmds.spawn(
        r::UiNode{}, r::Parent{ state.ptr->root },
        r::Style{ .width = 0.f, .height = 0.f, .width_pct = 100.f, .height_pct = 100.f, .background = {0,0,0,0}, .z_index = 1, .direction = r::LayoutDirection::Column, .justify = r::JustifyContent::Start, .align = r::AlignItems::Stretch, .gap = 0.f },
        r::ComputedLayout{}, r::Visibility::Visible);
    const r::ecs::Entity vbox_id = vbox.id();

    cmds.spawn(r::UiNode{}, r::Parent{ vbox_id }, r::Style{ .width = 0.f, .height = 0.f, .width_pct = 100.f, .height_pct = 15.f, .background = {0,0,0,0}, .margin = 0.001f }, r::ComputedLayout{}, r::Visibility::Visible);

    /* Header row above the panel (aligned to 60% center column) */
    auto header_row = cmds.spawn(
        r::UiNode{}, r::Parent{ vbox_id },
        r::Style{ .width = 0.f, .height = 56.f, .width_pct = 100.f, .height_pct = -1.f, .background = {0,0,0,0}, .z_index = 2, .margin = 0.f, .padding = 0.f, .direction = r::LayoutDirection::Row, .justify = r::JustifyContent::Start, .align = r::AlignItems::Center, .align_self = r::AlignSelf::Auto, .position = r::PositionType::Relative, .offset_x = 0.f, .offset_y = 0.f, .gap = 0.f, .clip_children = false, .overflow_clip = false, .border_thickness = 0.f, .border_color = {0,0,0,255} },
        r::ComputedLayout{}, r::Visibility::Visible);
    const r::ecs::Entity header_row_id = header_row.id();
    cmds.spawn(r::UiNode{}, r::Parent{ header_row_id }, r::Style{ .width = 0.f, .height = 0.f, .width_pct = 20.f, .height_pct = 100.f, .background = {0,0,0,0}, .z_index = 2, .margin = 0.f, .padding = 0.f, .direction = r::LayoutDirection::Column, .justify = r::JustifyContent::Start, .align = r::AlignItems::Stretch, .align_self = r::AlignSelf::Auto, .position = r::PositionType::Relative, .offset_x = 0.f, .offset_y = 0.f, .gap = 0.f, .clip_children = false, .overflow_clip = false, .border_thickness = 0.f, .border_color = {0,0,0,255} }, r::ComputedLayout{}, r::Visibility::Visible);
    auto header_center = cmds.spawn(
        r::UiNode{}, r::Parent{ header_row_id },
        r::Style{ .width = 0.f, .height = 0.f, .width_pct = 60.f, .height_pct = 100.f, .background = {0,0,0,0}, .z_index = 2, .margin = 0.f, .padding = 0.f, .direction = r::LayoutDirection::Column, .justify = r::JustifyContent::Center, .align = r::AlignItems::Center, .align_self = r::AlignSelf::Auto, .position = r::PositionType::Relative, .offset_x = 0.f, .offset_y = 0.f, .gap = 0.f, .clip_children = false, .overflow_clip = false, .border_thickness = 0.f, .border_color = {0,0,0,255} },
        r::ComputedLayout{}, r::Visibility::Visible);
    const r::ecs::Entity header_center_id = header_center.id();
    cmds.spawn(
        r::UiNode{}, r::Parent{ header_center_id },
        r::Style{ .width = 0.f, .height = 48.f, .width_pct = 100.f, .height_pct = -1.f, .background = {0,0,0,0}, .z_index = 3, .margin = 6.f, .padding = 0.f, .direction = r::LayoutDirection::Column, .justify = r::JustifyContent::Center, .align = r::AlignItems::Center, .align_self = r::AlignSelf::Auto, .position = r::PositionType::Relative, .offset_x = 0.f, .offset_y = 0.f, .gap = 0.f, .clip_children = false, .overflow_clip = false, .border_thickness = 0.f, .border_color = {0,0,0,255} },
        r::UiText{ .content = std::string("R-Engine — Menu Demo"), .font_size = 28, .wrap_width = 0.f, .color = {180,220,255,255} },
        r::ComputedLayout{}, r::Visibility::Visible);
    cmds.spawn(r::UiNode{}, r::Parent{ header_row_id }, r::Style{ .width = 0.f, .height = 0.f, .width_pct = 20.f, .height_pct = 100.f, .background = {0,0,0,0}, .z_index = 2, .margin = 0.f, .padding = 0.f, .direction = r::LayoutDirection::Column, .justify = r::JustifyContent::Start, .align = r::AlignItems::Stretch, .align_self = r::AlignSelf::Auto, .position = r::PositionType::Relative, .offset_x = 0.f, .offset_y = 0.f, .gap = 0.f, .clip_children = false, .overflow_clip = false, .border_thickness = 0.f, .border_color = {0,0,0,255} }, r::ComputedLayout{}, r::Visibility::Visible);

    auto mid_row = cmds.spawn(
        r::UiNode{}, r::Parent{ vbox_id },
        r::Style{ .width = 0.f, .height = 0.f, .width_pct = 100.f, .height_pct = 70.f, .background = {0,0,0,0}, .margin = 0.001f, .direction = r::LayoutDirection::Row, .align = r::AlignItems::Stretch, .gap = 0.f },
        r::ComputedLayout{}, r::Visibility::Visible);
    const r::ecs::Entity mid_row_id = mid_row.id();

    cmds.spawn(r::UiNode{}, r::Parent{ mid_row_id }, r::Style{ .width = 0.f, .height = 0.f, .width_pct = 20.f, .height_pct = 100.f, .background = {0,0,0,0}, .margin = 0.001f }, r::ComputedLayout{}, r::Visibility::Visible);

    auto panel = cmds.spawn(
        r::UiNode{}, r::Parent{ mid_row_id },
        r::Style{ .width = 0.f, .height = 0.f, .width_pct = 60.f, .height_pct = 100.f, .background = {0,0,0,0}, .z_index = 2, .margin = 0.001f, .padding = 12.f, .direction = r::LayoutDirection::Column, .justify = r::JustifyContent::Start, .align = r::AlignItems::Stretch, .gap = 12.f, .border_thickness = 2.f, .border_color = {160,160,160,255} },
        r::ComputedLayout{}, r::Visibility::Visible);
    const r::ecs::Entity panel_id = panel.id();

    cmds.spawn(r::UiNode{}, r::Parent{ mid_row_id }, r::Style{ .width = 0.f, .height = 0.f, .width_pct = 20.f, .height_pct = 100.f, .background = {0,0,0,0}, .margin = 0.001f }, r::ComputedLayout{}, r::Visibility::Visible);

    cmds.spawn(r::UiNode{}, r::Parent{ vbox_id }, r::Style{ .width = 0.f, .height = 0.f, .width_pct = 100.f, .height_pct = 15.f, .background = {0,0,0,0}, .margin = 0.001f }, r::ComputedLayout{}, r::Visibility::Visible);

    /* Title now lives in header_row above the panel */

    /* MAIN SCREEN */
    auto main_panel = cmds.spawn(
        r::UiNode{}, r::Parent{ panel_id },
        r::Style{ .width = 0.f, .height = 0.f, .width_pct = 100.f, .height_pct = -1.f, .background = {0,0,0,0}, .margin = 0.001f, .direction = r::LayoutDirection::Column, .justify = r::JustifyContent::Start, .align = r::AlignItems::Stretch, .gap = 10.f },
        r::ComputedLayout{}, r::Visibility::Visible);
    state.ptr->main_panel = main_panel.id();

    auto mk_btn = [&](const char *label) {
        return cmds.spawn(
            r::UiNode{}, r::Parent{ state.ptr->main_panel }, r::UiButton{},
            r::Style{ .width = 0.f, .height = 48.f, .width_pct = 60.f, .min_width = 220.f, .background = {0,0,0,0}, .z_index = 5, .margin = 6.f, .padding = 6.f, .direction = r::LayoutDirection::Column, .justify = r::JustifyContent::Center, .align = r::AlignItems::Center },
            r::UiText{ .content = std::string(label), .font_size = 22, .wrap_width = 0.f, .color = {0,0,0,0} },
            r::ComputedLayout{}, r::Visibility::Visible);
    };
    state.ptr->btn_play    = mk_btn("Play").id();
    state.ptr->btn_options = mk_btn("Options").id();
    state.ptr->btn_credits = mk_btn("Credits").id();
    state.ptr->btn_quit    = mk_btn("Quit").id();

    /* OPTIONS SCREEN */
    auto options = cmds.spawn(
        r::UiNode{}, r::Parent{ panel_id },
        r::Style{ .width = 0.f, .height = 0.f, .width_pct = 100.f, .background = {0,0,0,0}, .margin = 0.001f, .direction = r::LayoutDirection::Column, .justify = r::JustifyContent::Start, .align = r::AlignItems::Stretch, .gap = 8.f },
        r::ComputedLayout{}, r::Visibility::Collapsed);
    state.ptr->options_panel = options.id();

    cmds.spawn(r::UiNode{}, r::Parent{ state.ptr->options_panel }, r::Style{ .width = 0.f, .height = 40.f, .background = {0,0,0,0}, .margin = 6.f }, r::UiText{ .content = std::string("Options"), .font_size = 24, .color = {180, 220, 255, 255} }, r::ComputedLayout{}, r::Visibility::Visible);

    state.ptr->btn_toggle_theme = cmds.spawn(
        r::UiNode{}, r::Parent{ state.ptr->options_panel }, r::UiButton{},
        r::Style{ .width = 0.f, .height = 44.f, .width_pct = 50.f, .min_width = 200.f, .background = {0,0,0,0}, .margin = 6.f, .direction = r::LayoutDirection::Column, .justify = r::JustifyContent::Center, .align = r::AlignItems::Center },
        r::UiText{ .content = std::string("Toggle Theme (T)"), .font_size = 20, .color = {0,0,0,0} },
        r::ComputedLayout{}, r::Visibility::Visible
    ).id();

    state.ptr->btn_back_options = cmds.spawn(
        r::UiNode{}, r::Parent{ state.ptr->options_panel }, r::UiButton{},
        r::Style{ .width = 0.f, .height = 44.f, .width_pct = 40.f, .min_width = 180.f, .background = {0,0,0,0}, .margin = 6.f, .direction = r::LayoutDirection::Column, .justify = r::JustifyContent::Center, .align = r::AlignItems::Center },
        r::UiText{ .content = std::string("Back"), .font_size = 20, .color = {0,0,0,0} },
        r::ComputedLayout{}, r::Visibility::Visible
    ).id();

    /* CREDITS SCREEN */
    auto credits = cmds.spawn(
        r::UiNode{}, r::Parent{ panel_id },
        r::Style{ .width = 0.f, .height = 0.f, .width_pct = 100.f, .background = {0,0,0,0}, .margin = 0.001f, .direction = r::LayoutDirection::Column, .justify = r::JustifyContent::Start, .align = r::AlignItems::Stretch, .gap = 8.f },
        r::ComputedLayout{}, r::Visibility::Collapsed);
    state.ptr->credits_panel = credits.id();

    cmds.spawn(r::UiNode{}, r::Parent{ state.ptr->credits_panel }, r::Style{ .width = 0.f, .height = 36.f, .background = {0,0,0,0}, .margin = 6.f }, r::UiText{ .content = std::string("Credits"), .font_size = 24, .color = {180,220,255,255} }, r::ComputedLayout{}, r::Visibility::Visible);
    cmds.spawn(
        r::UiNode{}, r::Parent{ state.ptr->credits_panel },
        r::Style{ .width = 0.f, .height = 140.f, .width_pct = 100.f, .background = {0,0,0,0}, .margin = 6.f },
        r::UiText{ .content = std::string("R-Engine UI Plugin Demo\nMade with raylib.\n(UI nodes, buttons, images, scroll, focus, hover.)"), .font_size = 18, .wrap_width = 640.f, .color = {0,0,0,0} },
        r::ComputedLayout{}, r::Visibility::Visible);

    state.ptr->btn_back_credits = cmds.spawn(
        r::UiNode{}, r::Parent{ state.ptr->credits_panel }, r::UiButton{},
        r::Style{ .width = 0.f, .height = 44.f, .width_pct = 40.f, .min_width = 180.f, .background = {0,0,0,0}, .margin = 6.f, .direction = r::LayoutDirection::Column, .justify = r::JustifyContent::Center, .align = r::AlignItems::Center },
        r::UiText{ .content = std::string("Back"), .font_size = 20, .color = {0,0,0,0} },
        r::ComputedLayout{}, r::Visibility::Visible
    ).id();

    /* LEVEL SELECT SCREEN */
    auto levels = cmds.spawn(
        r::UiNode{}, r::Parent{ panel_id },
        r::Style{ .width = 0.f, .height = 0.f, .width_pct = 100.f, .background = {0,0,0,0}, .margin = 0.001f, .direction = r::LayoutDirection::Column, .justify = r::JustifyContent::Start, .align = r::AlignItems::Stretch, .gap = 8.f },
        r::ComputedLayout{}, r::Visibility::Collapsed);
    state.ptr->levels_panel = levels.id();

    cmds.spawn(r::UiNode{}, r::Parent{ state.ptr->levels_panel }, r::Style{ .width = 0.f, .height = 36.f, .background = {0,0,0,0}, .margin = 6.f }, r::UiText{ .content = std::string("Select a Level"), .font_size = 24, .color = {0,0,0,0} }, r::ComputedLayout{}, r::Visibility::Visible);

    auto scroll_area = cmds.spawn(
        r::UiNode{}, r::Parent{ state.ptr->levels_panel }, r::UiScroll{ .x = 0.f, .y = 0.f },
        r::Style{ .width = 0.f, .height = 260.f, .width_pct = 100.f, .height_pct = -1.f, .min_width = 0.f, .max_width = 0.f, .min_height = 0.f, .max_height = 0.f, .background = {0,0,0,0}, .z_index = 2, .margin = 6.f, .padding = 4.f, .direction = r::LayoutDirection::Column, .justify = r::JustifyContent::Start, .align = r::AlignItems::Stretch, .align_self = r::AlignSelf::Auto, .position = r::PositionType::Relative, .offset_x = 0.f, .offset_y = 0.f, .gap = 0.f, .clip_children = true, .overflow_clip = true, .border_thickness = 2.f, .border_color = {160,160,160,255} },
        r::ComputedLayout{}, r::Visibility::Visible);
    const r::ecs::Entity scroll_parent = scroll_area.id();

    state.ptr->level_items.clear();
    for (int i = 0; i < 15; ++i) {
        auto item = cmds.spawn(
            r::UiNode{}, r::Parent{ scroll_parent }, r::UiButton{},
            r::Style{ .width = 0.f, .height = 40.f, .width_pct = 100.f, .background = {0,0,0,0}, .z_index = 2, .margin = 4.f, .direction = r::LayoutDirection::Row, .justify = r::JustifyContent::Start, .align = r::AlignItems::Center },
            r::UiText{ .content = std::string("Level ") + std::to_string(i+1), .font_size = 18, .wrap_width = 0.f, .color = {0,0,0,0} },
            r::ComputedLayout{}, r::Visibility::Visible);
        state.ptr->level_items.push_back(item.id());
    }

    state.ptr->btn_back_levels = cmds.spawn(
        r::UiNode{}, r::Parent{ state.ptr->levels_panel }, r::UiButton{},
        r::Style{ .width = 0.f, .height = 44.f, .width_pct = 40.f, .min_width = 180.f, .background = {0,0,0,0}, .margin = 6.f, .direction = r::LayoutDirection::Column, .justify = r::JustifyContent::Center, .align = r::AlignItems::Center },
        r::UiText{ .content = std::string("Back"), .font_size = 20, .color = {0,0,0,0} },
        r::ComputedLayout{}, r::Visibility::Visible
    ).id();
}

/* Toggle theme (T key or button) */
static void theme_toggle_system(r::ecs::Res<r::UserInput> ui, r::ecs::Res<r::InputMap> map, r::ecs::ResMut<r::UiTheme> theme, r::ecs::ResMut<r::UiPluginConfig> cfg)
{
    if (map.ptr->isActionPressed("ToggleTheme", *ui.ptr)) {
        toggle_theme_now(theme, cfg);
    }
}

/* Bind inputs: theme toggle + back */
static void setup_controls(r::ecs::ResMut<r::UiPluginConfig> cfg, r::ecs::ResMut<r::InputMap> map)
{
    cfg.ptr->overlay_text = "UI Menu Demo";
    map.ptr->bindAction("ToggleTheme", r::KEYBOARD, KEY_T);
    map.ptr->bindAction("Back", r::KEYBOARD, KEY_ESCAPE);
}

/* Scroll with mouse wheel for all UiScroll containers */
static void scroll_wheel_system(r::ecs::Res<r::UserInput> ui, r::ecs::Query<r::ecs::Mut<r::UiScroll>> q)
{
    (void)ui;
    float wheel = GetMouseWheelMove();
    if (wheel == 0.f) return;
    for (auto [scroll] : q) {
        scroll.ptr->y -= wheel * 40.f;
        if (scroll.ptr->y < 0.f) scroll.ptr->y = 0.f;
    }
}

/* Handle menu interactions via UiEvents */
static void menu_logic_system(
    r::ecs::Commands &cmds,
    r::ecs::Res<r::UiEvents> events,
    r::ecs::Res<r::UserInput> ui,
    r::ecs::Res<r::InputMap> map,
    r::ecs::ResMut<r::UiTheme> theme,
    r::ecs::ResMut<r::UiPluginConfig> cfg,
    r::ecs::ResMut<MenuState> state)
{
    auto show_screen = [&](MenuScreen s) {
        state.ptr->screen = s;
        set_panel_visibility(cmds, state.ptr->main_panel,   (s == MenuScreen::Main)    ? r::Visibility::Visible : r::Visibility::Collapsed);
        set_panel_visibility(cmds, state.ptr->options_panel,(s == MenuScreen::Options) ? r::Visibility::Visible : r::Visibility::Collapsed);
        set_panel_visibility(cmds, state.ptr->credits_panel,(s == MenuScreen::Credits) ? r::Visibility::Visible : r::Visibility::Collapsed);
        set_panel_visibility(cmds, state.ptr->levels_panel, (s == MenuScreen::Levels)  ? r::Visibility::Visible : r::Visibility::Collapsed);
        switch (s) {
            case MenuScreen::Main:    cfg.ptr->overlay_text = "Menu: Main"; break;
            case MenuScreen::Options: cfg.ptr->overlay_text = "Menu: Options"; break;
            case MenuScreen::Credits: cfg.ptr->overlay_text = "Menu: Credits"; break;
            case MenuScreen::Levels:  cfg.ptr->overlay_text = "Menu: Levels"; break;
        }
    };

    /* Click handling */
    for (auto e : events.ptr->clicked) {
        if (e == state.ptr->btn_play) {
            show_screen(MenuScreen::Levels);
        } else if (e == state.ptr->btn_options) {
            show_screen(MenuScreen::Options);
        } else if (e == state.ptr->btn_credits) {
            show_screen(MenuScreen::Credits);
        } else if (e == state.ptr->btn_quit) {
            r::Application::quit = true;
        } else if (e == state.ptr->btn_back_options || e == state.ptr->btn_back_credits || e == state.ptr->btn_back_levels) {
            show_screen(MenuScreen::Main);
        } else if (e == state.ptr->btn_toggle_theme) {
            toggle_theme_now(theme, cfg);
        } else {
            /* Level selection */
            for (size_t i = 0; i < state.ptr->level_items.size(); ++i) {
                if (state.ptr->level_items[i] == e) {
                    state.ptr->selected_level = static_cast<int>(i);
                    cfg.ptr->overlay_text = std::string("Selected Level ") + std::to_string(i + 1);
                    /* Visual feedback: disable selected, enable others */
                    for (size_t j = 0; j < state.ptr->level_items.size(); ++j) {
                        cmds.add_component(state.ptr->level_items[j], r::UiButton{ .disabled = (j == i) });
                    }
                    break;
                }
            }
        }
    }

    /* ESC -> Back to main */
    if (map.ptr->isActionPressed("Back", *ui.ptr)) {
        if (state.ptr->screen != MenuScreen::Main) show_screen(MenuScreen::Main);
    }
}

int main()
{
    r::Application{}
        .add_plugins(
            r::DefaultPlugins{}
                .set(r::WindowPlugin{r::WindowPluginConfig{
                    .size = {960, 540},
                    .title = "R-Engine UI Demo — Faux Menu",
                    .cursor = r::WindowCursorState::Visible,
                }})
        )
        .add_plugins(r::UiPlugin{})
        .insert_resource(MenuState{})
        .add_systems(r::Schedule::STARTUP, setup_controls, build_menu_ui)
        .add_systems(r::Schedule::UPDATE, theme_toggle_system, menu_logic_system, scroll_wheel_system)
        .run();
}
