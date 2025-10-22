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
    // Désactiver l'overlay de debug
    cfg.ptr->show_debug_overlay = false;
    
    // Configuration du thème pour les boutons
    theme.ptr->button.bg_normal = r::Color{0, 36, 48, 255};       // Fond #002430
    theme.ptr->button.bg_hover = r::Color{98, 221, 255, 100};   // Fond semi-transparent au survol
    theme.ptr->button.bg_pressed = r::Color{98, 221, 255, 150}; // Fond plus visible au clic
    theme.ptr->button.bg_disabled = r::Color{50, 50, 50, 255};
    
    theme.ptr->button.border_normal = r::Color{98, 221, 255, 255};  // Contour cyan
    theme.ptr->button.border_hover = r::Color{98, 221, 255, 255};   // Contour cyan
    theme.ptr->button.border_pressed = r::Color{98, 221, 255, 255}; // Contour cyan
    theme.ptr->button.border_disabled = r::Color{100, 100, 100, 255};
    
    theme.ptr->button.border_thickness = 2.f;
    theme.ptr->button.text = r::Color{98, 221, 255, 255};  // Texte cyan
}

static void build_main_menu(r::ecs::Commands &cmds, r::ecs::Res<r::WindowPluginConfig> win)
{
    (void) win;

    // Fond noir avec le logo au centre
    cmds.spawn(r::UiNode{}, 
            r::Style{
                .width_pct = 100.f, 
                .height_pct = 100.f, 
                .background = r::Color{0, 0, 0, 255},  // Fond noir
                .margin = 0.f,  // Pas de marge
                .padding = 0.f,  // Pas de padding
                .direction = r::LayoutDirection::Column,
                .justify = r::JustifyContent::Center,
                .align = r::AlignItems::Center,
                .gap = 10.f  // Espacement réduit entre les éléments
            },
            r::ComputedLayout{}, 
            r::Visibility::Visible)
        .with_children([&](r::ecs::ChildBuilder &parent) {
            // Titre R-Type en pleine largeur
            parent.spawn(r::UiNode{},
                r::Style{
                    .height = 200.f,     // Hauteur augmentée pour voir l'image complète
                    .width_pct = 100.f,  // Toute la largeur disponible
                    .background = r::Color{0, 0, 0, 1},  // Quasi-transparent
                    .margin = 0.f,  // Pas de marge
                    .padding = 0.f  // Pas de padding
                },
                r::UiImage{
                    .path = "assets/r-type_title.png",
                    .tint = r::Color{255, 255, 255, 255},
                    .keep_aspect = true  // Garder le ratio
                },
                r::ComputedLayout{}, 
                r::Visibility::Visible);

            // Bouton Play
            parent.spawn(r::UiNode{}, 
                r::UiButton{}, 
                MenuButton{MenuAction::Play},
                r::Style{
                    .width = 280.f,  // Largeur augmentée
                    .height = 45.f,
                    .direction = r::LayoutDirection::Column,
                    .justify = r::JustifyContent::Center,
                    .align = r::AlignItems::Center
                },
                r::UiText{
                    .content = std::string("Play"),
                    .font_size = 22
                },
                r::ComputedLayout{}, 
                r::Visibility::Visible);

            // Bouton Options
            parent.spawn(r::UiNode{}, 
                r::UiButton{}, 
                MenuButton{MenuAction::Options},
                r::Style{
                    .width = 280.f,  // Largeur augmentée
                    .height = 45.f,
                    .direction = r::LayoutDirection::Column,
                    .justify = r::JustifyContent::Center,
                    .align = r::AlignItems::Center
                },
                r::UiText{
                    .content = std::string("Options"),
                    .font_size = 22
                },
                r::ComputedLayout{}, 
                r::Visibility::Visible);

            // Bouton Quit
            parent.spawn(r::UiNode{}, 
                r::UiButton{}, 
                MenuButton{MenuAction::Quit},
                r::Style{
                    .width = 280.f,  // Largeur augmentée
                    .height = 45.f,
                    .direction = r::LayoutDirection::Column,
                    .justify = r::JustifyContent::Center,
                    .align = r::AlignItems::Center
                },
                r::UiText{
                    .content = std::string("Quit"),
                    .font_size = 22
                },
                r::ComputedLayout{}, 
                r::Visibility::Visible);
        });
}

static void menu_logic_system(r::ecs::EventReader<r::UiClick> click_reader, 
    r::ecs::Query<r::ecs::Ref<MenuButton>> buttons)
{
    for (const auto &click : click_reader) {
        const auto clicked = click.entity;
        if (clicked == r::ecs::NULL_ENTITY) {
            continue;
        }

        MenuAction action = MenuAction::None;
        for (auto it = buttons.begin(); it != buttons.end(); ++it) {
            auto [btn] = *it;
            if (static_cast<r::ecs::Entity>(it.entity()) == clicked && btn.ptr) {
                action = btn.ptr->action;
                break;
            }
        }

        switch (action) {
            case MenuAction::Play:
                // TODO: Démarrer le jeu
                r::Logger::info("Play button clicked!");
                break;
            case MenuAction::Options:
                // TODO: Ouvrir les options
                r::Logger::info("Options button clicked!");
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
            .title = "R-Type - Main Menu",
            .cursor = r::WindowCursorState::Visible,
        }}))
        .add_plugins(r::UiPlugin{})
        .add_systems<setup_theme, build_main_menu>(r::Schedule::STARTUP)
        .add_systems<menu_logic_system>(r::Schedule::UPDATE)
        .after<r::ui::pointer_system>()
        .run();
}
