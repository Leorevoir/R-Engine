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

static void setup_theme(r::ecs::ResMut<r::UiTheme> theme, r::ecs::ResMut<r::UiPluginConfig> cfg)
{
    cfg.ptr->show_debug_overlay = false;
    
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

static void build_main_menu(r::ecs::Commands cmds, r::ecs::Res<r::AssetServer> assets)
{
    auto root = cmds.spawn(r::UiNode{
        .name = "MenuRoot",
        .style = r::Style{
            .width = 960.f,
            .height = 540.f,
            .bg_color = r::Color{0, 0, 0, 255},
            .justify_content = r::ui::JustifyContent::Center,
            .align_items = r::ui::AlignItems::Center,
            .flex_direction = r::ui::FlexDirection::Column,
            .gap = 30.f,
        },
    });
    
    root.add_child(cmds.spawn(r::UiImage{
        .style = r::Style{
            .height = 200.f,
            .width_pct = 100.f,
            .margin = 0.f,
            .padding = 0.f,
        },
        .texture = assets.ptr->load_image("assets/r-type_title.png"),
        .keep_aspect = false,
    }));
    
    root.add_child(MenuButton{
        cmds.spawn(r::UiButton{
            .action = MenuAction::Play,
            .style = r::Style{
                .width = 280.f,
                .height = 45.f,
            },
            .text = r::UiText{.text = "Play"},
        })
    });
    
    root.add_child(MenuButton{
        cmds.spawn(r::UiButton{
            .action = MenuAction::Options,
            .style = r::Style{
                .width = 280.f,
                .height = 45.f,
            },
            .text = r::UiText{.text = "Options"},
        })
    });
    
    root.add_child(MenuButton{
        cmds.spawn(r::UiButton{
            .action = MenuAction::Quit,
            .style = r::Style{
                .width = 280.f,
                .height = 45.f,
            },
            .text = r::UiText{.text = "Quit"},
        })
    });
}

static void menu_logic_system(r::ecs::Query<MenuButton, const r::UiButton> buttons,
                               r::ecs::ResMut<r::AppExit> app_exit)
{
    for (auto [button, ui_button] : buttons.iter()) {
        if (ui_button.ptr->just_clicked()) {
            switch (button.ptr->action) {
            case MenuAction::Play:
                std::cout << "Play button clicked" << std::endl;
                break;
            case MenuAction::Options:
                std::cout << "Options button clicked" << std::endl;
                break;
            case MenuAction::Quit:
                std::cout << "Quit button clicked" << std::endl;
                app_exit.ptr->request_exit();
                break;
            default:
                break;
            }
        }
    }
}

int main()
{
    r::Application{}
        .add_plugins(r::DefaultPlugins{}.set(r::WindowPlugin{r::WindowPluginConfig{
            .size = {960, 540},
            .title = "R-Type - Main Menu",
            .cursor = r::WindowCursorState::Visible,
        }}))
        .add_plugins(r::UiPlugin{})
        .add_systems<setup_theme, build_main_menu>(r::Schedule::STARTUP)
        .add_systems<menu_logic_system>(r::Schedule::UPDATE)
        .after<r::ui::pointer_system>()
        .before<r::ui::clear_click_state_system>()
        .run();
}
