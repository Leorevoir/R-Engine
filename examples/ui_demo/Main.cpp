#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/Plugins/Ui/Systems.hpp>
#include <R-Engine/Plugins/UiPlugin.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>
#include <R-Engine/UI/Button.hpp>
#include <R-Engine/UI/Components.hpp>
#include <R-Engine/UI/Events.hpp>
#include <R-Engine/UI/Image.hpp>
#include <R-Engine/UI/InputState.hpp>
#include <R-Engine/UI/Text.hpp>
#include <R-Engine/UI/Theme.hpp>

#include <string>

enum class MenuAction {
    None,
    Play,
    Options,
    Quit,
};

struct MenuButton {
        MenuAction action = MenuAction::None;
};

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

static void build_menu_ui(r::ecs::Commands &cmds, r::ecs::Res<r::WindowPluginConfig> win)
{
    (void) win;

    cmds.spawn(r::UiNode{}, r::Style{.width_pct = 100.f, .height_pct = 100.f, .background = r::Color{30, 30, 38, 255}}, r::ComputedLayout{},
            r::Visibility::Visible)
        .with_children([&](r::ecs::ChildBuilder &parent) {
            /* Center container */
            parent
                .spawn(r::UiNode{},
                    r::Style{.width_pct = 100.f,
                        .height_pct = 100.f,
                        .background = r::Color{0, 0, 0, 0},
                        .direction = r::LayoutDirection::Column,
                        .justify = r::JustifyContent::Center,
                        .align = r::AlignItems::Center,
                        .gap = 12.f},
                    r::ComputedLayout{}, r::Visibility::Visible)
                .with_children([&](r::ecs::ChildBuilder &center) {
                    /* Title */
                    center.spawn(r::UiNode{},
                        r::Style{.height = 64.f,
                            .width_pct = 80.f,
                            .min_width = 200.f,
                            .max_width = 480.f,
                            .background = r::Color{0, 0, 0, 0},
                            .order = 0,
                            .margin = 6.f,
                            .direction = r::LayoutDirection::Column,
                            .justify = r::JustifyContent::Center,
                            .align = r::AlignItems::Center},
                        r::UiText{.content = std::string("R-Type"),
                            .font_size = 40,
                            .wrap_width = 0.f,
                            .color = r::Color{180, 220, 255, 255}},
                        r::ComputedLayout{}, r::Visibility::Visible);

                    /* Buttons Panel */
                    center
                        .spawn(r::UiNode{},
                            r::Style{.height = 200.f,
                                .width_pct = 80.f,
                                .min_width = 200.f,
                                .max_width = 420.f,
                                .background = r::Color{0, 0, 0, 0},
                                .order = 1,
                                .direction = r::LayoutDirection::Column,
                                .justify = r::JustifyContent::Center,
                                .align = r::AlignItems::Center,
                                .gap = 10.f},
                            r::ComputedLayout{}, r::Visibility::Visible)
                        .with_children([&](r::ecs::ChildBuilder &panel) {
                            panel.spawn(r::UiNode{}, r::UiButton{}, MenuButton{MenuAction::Play},
                                r::Style{.height = 44.f,
                                    .width_pct = 90.f,
                                    .min_width = 180.f,
                                    .max_width = 360.f,
                                    .order = 0,
                                    .margin = 6.f,
                                    .direction = r::LayoutDirection::Column,
                                    .justify = r::JustifyContent::Center,
                                    .align = r::AlignItems::Center},
                                r::UiText{.content = std::string("Play"), .font_size = 22}, r::ComputedLayout{}, r::Visibility::Visible);

                            panel.spawn(r::UiNode{}, r::UiButton{}, MenuButton{MenuAction::Options},
                                r::Style{.height = 44.f,
                                    .width_pct = 90.f,
                                    .min_width = 180.f,
                                    .max_width = 360.f,
                                    .order = 1,
                                    .margin = 6.f,
                                    .direction = r::LayoutDirection::Column,
                                    .justify = r::JustifyContent::Center,
                                    .align = r::AlignItems::Center},
                                r::UiText{.content = std::string("Options"), .font_size = 22}, r::ComputedLayout{}, r::Visibility::Visible);

                            panel.spawn(r::UiNode{}, r::UiButton{}, MenuButton{MenuAction::Quit},
                                r::Style{.height = 44.f,
                                    .width_pct = 90.f,
                                    .min_width = 180.f,
                                    .max_width = 360.f,
                                    .order = 2,
                                    .margin = 6.f,
                                    .direction = r::LayoutDirection::Column,
                                    .justify = r::JustifyContent::Center,
                                    .align = r::AlignItems::Center},
                                r::UiText{.content = std::string("Quit"), .font_size = 22}, r::ComputedLayout{}, r::Visibility::Visible);
                        });

                    /* Scrollable Panel */
                    center
                        .spawn(r::UiNode{}, r::UiScroll{},
                            r::Style{.height = 220.f,
                                .width_pct = 80.f,
                                .min_width = 200.f,
                                .max_width = 420.f,
                                .background = r::Color{0, 0, 0, 0},
                                .order = 2,
                                .direction = r::LayoutDirection::Column,
                                .justify = r::JustifyContent::Start,
                                .align = r::AlignItems::Stretch,
                                .gap = 6.f,
                                .clip_children = true},
                            r::ComputedLayout{}, r::Visibility::Visible)
                        .with_children([&](r::ecs::ChildBuilder &scroll_panel) {
                            for (int i = 1; i <= 24; ++i) {
                                char buf[64];
                                snprintf(buf, sizeof(buf), "List item %02d", i);
                                scroll_panel.spawn(r::UiNode{},
                                    r::Style{.height = 28.f,
                                        .width_pct = 95.f,
                                        .background = r::Color{20, (u8) (20 + i * 2), (u8) (28 + i), 180},
                                        .order = i,
                                        .margin = 4.f,
                                        .padding = 6.f,
                                        .direction = r::LayoutDirection::Row,
                                        .justify = r::JustifyContent::Start,
                                        .align = r::AlignItems::Center},
                                    r::UiText{.content = std::string(buf), .font_size = 18}, r::ComputedLayout{}, r::Visibility::Visible);
                            }
                        });
                });
        })
        .id();
}

static void theme_toggle_system(r::ecs::Res<r::UserInput> ui, r::ecs::Res<r::InputMap> map, r::ecs::ResMut<r::UiTheme> theme,
    r::ecs::ResMut<r::UiPluginConfig> cfg)
{
    if (map.ptr->isActionPressed("ToggleTheme", *ui.ptr))
        toggle_theme_now(theme, cfg);
}

static void setup_controls(r::ecs::ResMut<r::UiPluginConfig> cfg, r::ecs::ResMut<r::InputMap> map)
{
    cfg.ptr->overlay_text = "R-Type Menu";
    map.ptr->bindAction("ToggleTheme", r::KEYBOARD, KEY_T);
}

static void menu_logic_system(r::ecs::EventReader<r::UiClick> click_reader, r::ecs::ResMut<r::UiPluginConfig> cfg,
    r::ecs::Query<r::ecs::Ref<MenuButton>> buttons)
{
    for (const auto &click : click_reader) {
        const auto clicked_entity = click.entity;
        if (clicked_entity == r::ecs::NULL_ENTITY) {
            continue;
        }

        MenuAction action = MenuAction::None;
        for (auto it = buttons.begin(); it != buttons.end(); ++it) {
            auto [btn] = *it;
            if (static_cast<r::ecs::Entity>(it.entity()) == clicked_entity && btn.ptr) {
                action = btn.ptr->action;
                break;
            }
        }

        switch (action) {
            case MenuAction::Play:
                cfg.ptr->overlay_text = "Play selected";
                break;
            case MenuAction::Options:
                cfg.ptr->overlay_text = "Options selected";
                break;
            case MenuAction::Quit:
                r::Application::quit.store(true, std::memory_order_relaxed);
                break;
            default:
                break;
        }
    }
}

int main()
{
    r::Application{}
        .add_plugins(r::DefaultPlugins{}.set(r::WindowPlugin{r::WindowPluginConfig{
            .size = {960, 540},
            .title = "R-Engine UI Demo — Faux Menu",
            .cursor = r::WindowCursorState::Visible,
        }}))
        .add_plugins(r::UiPlugin{})
        .add_systems<setup_controls, build_menu_ui>(r::Schedule::STARTUP)
        .add_systems<theme_toggle_system>(r::Schedule::UPDATE)
        .add_systems<menu_logic_system>(r::Schedule::UPDATE)
        .after<r::ui::pointer_system>()
        .run();
}
