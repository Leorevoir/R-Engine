#include <R-Engine/Application.hpp>
#include <R-Engine/Plugins/UiPlugin.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/UI/Fonts.hpp>
#include <R-Engine/UI/InputState.hpp>
#include <R-Engine/UI/Theme.hpp>
#include <R-Engine/UI/Events.hpp>
#include <R-Engine/UI/Textures.hpp>
#include <R-Engine/Plugins/Ui/Systems.hpp>

namespace r {

UiPlugin::UiPlugin(const UiPluginConfig &cfg) noexcept : _config(cfg) {}

void UiPlugin::build(Application &app)
{
    app
        .insert_resource(_config)
        .insert_resource(UiFonts{})
        .insert_resource(UiTheme{})
        .insert_resource(UiTextures{})
        .insert_resource(UiInputState{})
        .insert_resource(UiEvents{})
        .add_systems<ui::startup_system>(Schedule::STARTUP)
        .add_systems<ui::update_system>(Schedule::UPDATE)
        .add_systems<
            ui::compute_layout_system,
            ui::pointer_system,
            ui::scroll_input_system,
            ui::scroll_clamp_system,
            ui::keyboard_nav_system
        >(Schedule::UPDATE)
            .after<ui::update_system>()
        .add_systems<ui::clear_click_state_system>(Schedule::UPDATE)
            .after<ui::pointer_system>()
        .add_systems<ui::render_system>(Schedule::RENDER_2D)
        .add_systems<[](r::ecs::ResMut<r::UiTextures> tex, r::ecs::ResMut<r::UiFonts> fonts){
            if (IsWindowReady()) {
                for (auto &kv : tex.ptr->cache) { UnloadTexture(kv.second); }
                for (auto &kv : fonts.ptr->cache) { UnloadFont(kv.second); }
            }
            tex.ptr->cache.clear();
            fonts.ptr->cache.clear();
        }>(Schedule::SHUTDOWN);

    Logger::info("UiPlugin built");
}

}
