#include "menu.hpp"
#include "../components.hpp"
#include "../state.hpp"

#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/ECS/RunConditions.hpp>
#include <R-Engine/Plugins/Ui/Systems.hpp>
#include <R-Engine/Plugins/UiPlugin.hpp>
#include <R-Engine/UI/Button.hpp>
#include <R-Engine/UI/Components.hpp>
#include <R-Engine/UI/Events.hpp>
#include <R-Engine/UI/Image.hpp>
#include <R-Engine/UI/InputState.hpp>
#include <R-Engine/UI/Text.hpp>
#include <R-Engine/UI/Theme.hpp>

// clang-format off

/* ================================================================================= */
/* Menu Systems :: Helpers */
/* ================================================================================= */

static void create_menu_title(r::ecs::ChildBuilder& parent)
{
    parent.spawn(
        r::UiNode{},
        r::Style{.height = 200.f, .width_pct = 100.f, .background = r::Color{0, 0, 0, 1}, .margin = 0.f, .padding = 0.f},
        r::UiImage{.path = "examples/minimal-r_type/assets/textures/r-type_title.png", .tint = r::Color{255, 255, 255, 255}, .keep_aspect = true},
        r::ComputedLayout{},
        r::Visibility::Visible
    );
}

static void create_menu_button(r::ecs::ChildBuilder& parent, MenuButton::Action action, const std::string& text)
{
    parent.spawn(
        r::UiNode{}, r::UiButton{}, MenuButton{action},
        r::Style{
            .width = 280.f,
            .height = 45.f,
            .direction = r::LayoutDirection::Column,
            .justify = r::JustifyContent::Center,
            .align = r::AlignItems::Center
        },
        r::UiText{.content = text, .font_size = 22, .font_path = {}},
        r::ComputedLayout{},
        r::Visibility::Visible
    );
}

/* ================================================================================= */
/* Menu Systems */
/* ================================================================================= */

static void setup_ui_theme(r::ecs::ResMut<r::UiTheme> theme, r::ecs::ResMut<r::UiPluginConfig> cfg)
{
    cfg.ptr->show_debug_overlay = false;

    /* R-Type cyan theme (#62DDFF) */
    theme.ptr->button.bg_normal = r::Color{0, 36, 48, 255};
    theme.ptr->button.bg_hover = r::Color{98, 221, 255, 100};
    theme.ptr->button.bg_pressed = r::Color{98, 221, 255, 150};
    theme.ptr->button.bg_disabled = r::Color{50, 50, 50, 255};

    theme.ptr->button.border_normal = r::Color{98, 221, 255, 255};
    theme.ptr->button.border_hover = r::Color{98, 221, 255, 255};
    theme.ptr->button.border_pressed = r::Color{98, 221, 255, 255};
    theme.ptr->button.border_disabled = r::Color{100, 100, 100, 255};

    theme.ptr->button.border_thickness = 2.f;
    theme.ptr->button.text = r::Color{98, 221, 255, 255};
}

static void build_main_menu(r::ecs::Commands& cmds)
{
    cmds.spawn(
        MenuRoot{}, r::UiNode{},
        r::Style{
            .width_pct = 100.f,
            .height_pct = 100.f,
            .background = r::Color{0, 0, 0, 255},
            .margin = 0.f,
            .padding = 0.f,
            .direction = r::LayoutDirection::Column,
            .justify = r::JustifyContent::Center,
            .align = r::AlignItems::Center,
            .gap = 10.f
        },
        r::ComputedLayout{},
        r::Visibility::Visible
    ).with_children([&](r::ecs::ChildBuilder& parent) {
        create_menu_title(parent);
        create_menu_button(parent, MenuButton::Action::Play, "Play");
        create_menu_button(parent, MenuButton::Action::Options, "Options");
        create_menu_button(parent, MenuButton::Action::Quit, "Quit");
    });
}

static void menu_button_handler(r::ecs::EventReader<r::UiClick> click_reader, r::ecs::Query<r::ecs::Ref<MenuButton>> buttons,
                                r::ecs::ResMut<r::NextState<GameState>> next_state)
{
    for (const auto &click : click_reader) {
        const r::ecs::Entity clicked_entity = click.entity;
        if (clicked_entity == r::ecs::NULL_ENTITY) {
            continue;
        }

        MenuButton::Action action = MenuButton::Action::None;
        for (auto it = buttons.begin(); it != buttons.end(); ++it) {
            auto [btn] = *it;
            if (it.entity() == clicked_entity && btn.ptr) {
                action = btn.ptr->action;
                break;
            }
        }

        switch (action) {
            case MenuButton::Action::Play:
                r::Logger::info("Starting game...");
                next_state.ptr->set(GameState::EnemiesBattle);
                break;
            case MenuButton::Action::Options:
                r::Logger::info("Options clicked (not implemented)");
                break;
            case MenuButton::Action::Quit:
                r::Logger::info("Quitting game...");
                r::Application::quit.store(true, std::memory_order_relaxed);
                break;
            default:
                break;
        }
    }
}

static void cleanup_menu(r::ecs::Commands& cmds, r::ecs::Query<r::ecs::Ref<MenuRoot>> menu_entities)
{
    for (auto it = menu_entities.begin(); it != menu_entities.end(); ++it) {
        cmds.despawn(it.entity());
    }
}

static void show_game_over_ui(r::ecs::Commands& cmds)
{
    cmds.spawn(
        GameOverRoot{}, r::UiNode{},
        r::Style{
            .width_pct = 100.f,
            .height_pct = 100.f,
            .background = r::Color{0, 0, 0, 150}, /* Semi-transparent black background */
            .direction = r::LayoutDirection::Column,
            .justify = r::JustifyContent::Center,
            .align = r::AlignItems::Center,
            .gap = 20.f
        },
        r::ComputedLayout{},
        r::Visibility::Visible
    )
    .with_children([&](r::ecs::ChildBuilder& parent) {
        parent.spawn(r::UiNode{}, r::UiText{.content = "GAME OVER", .font_size = 80, .color = {255, 50, 50, 255}, .font_path = {}},
                     r::Style{.height = 90.f}, r::ComputedLayout{}, r::Visibility::Visible);
        parent.spawn(r::UiNode{}, r::UiText{.content = "Press ENTER to Restart", .font_size = 30, .color = {200, 200, 200, 255}, .font_path = {}},
                     r::Style{.height = 40.f}, r::ComputedLayout{}, r::Visibility::Visible);
    });
}

static void cleanup_game_over_ui(r::ecs::Commands& cmds, r::ecs::Query<r::ecs::With<GameOverRoot>> query)
{
    for (auto it = query.begin(); it != query.end(); ++it) {
        cmds.despawn(it.entity());
    }
}

static void game_over_system(r::ecs::Res<r::UserInput> user_input, r::ecs::ResMut<r::NextState<GameState>> next_state)
{
    if (user_input.ptr->isKeyPressed(KEY_ESCAPE)) {
        r::Application::quit.store(true, std::memory_order_relaxed);
    }

    if (user_input.ptr->isKeyPressed(KEY_ENTER)) {
        r::Logger::info("Restarting game...");
        next_state.ptr->set(GameState::EnemiesBattle);
    }
}

void MenuPlugin::build(r::Application& app)
{
    app
        /* Global UI setup */
        .add_systems<setup_ui_theme>(r::Schedule::STARTUP)

        /* Main Menu State */
        .add_systems<build_main_menu>(r::OnEnter{GameState::MainMenu})
        .add_systems<menu_button_handler>(r::Schedule::UPDATE)
            .run_if<r::run_conditions::on_event<r::UiClick>>()
        .add_systems<cleanup_menu>(r::OnExit{GameState::MainMenu})

        /* GameOver State */
        .add_systems<show_game_over_ui>(r::OnEnter{GameState::GameOver})
        .add_systems<cleanup_game_over_ui>(r::OnExit{GameState::GameOver})
        .add_systems<game_over_system>(r::Schedule::UPDATE)
        .run_if<r::run_conditions::in_state<GameState::GameOver>>();
}
// clang-format on
