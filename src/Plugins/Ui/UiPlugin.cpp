#include <R-Engine/Plugins/UiPlugin.hpp>

#include <R-Engine/Application.hpp>
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
        .add_systems(Schedule::STARTUP, ui::startup_system)
        .add_systems(Schedule::UPDATE,
            ui::update_system,
            ui::compute_layout_system,
            ui::pointer_system,
            ui::scroll_input_system,
            ui::scroll_clamp_system,
            ui::keyboard_nav_system)
        .add_systems(Schedule::RENDER, ui::render_system)
        .add_systems(Schedule::SHUTDOWN, [](r::ecs::ResMut<r::UiTextures> tex, r::ecs::ResMut<r::UiFonts> fonts){
            if (IsWindowReady()) {
                for (auto &kv : tex.ptr->cache) { UnloadTexture(kv.second); }
                for (auto &kv : fonts.ptr->cache) { UnloadFont(kv.second); }
            }
            tex.ptr->cache.clear();
            fonts.ptr->cache.clear();
        });

    Logger::info("UiPlugin built");
}

} // namespace r
