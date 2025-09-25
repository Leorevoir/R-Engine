#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Plugins/ServerPlugin.hpp>

#include "ServerPluginImpl.hpp"

r::ServerPlugin::ServerPlugin(const ServerPluginConfig &config) : _config(config)
{
    /* ctor */
}

// clang-format off

void r::ServerPlugin::build(Application &app)
{
    const auto impl = std::make_shared<ServerPluginImpl>();

    app
        .insert_resource(_config)
        .add_systems(Schedule::STARTUP, [impl](ecs::Res<ServerPluginConfig> config) {
            impl->start(config);
        })
        .add_systems(Schedule::UPDATE, [impl]() {
            impl->update();
        })
        .add_systems(Schedule::SHUTDOWN, [impl]() {
            impl->stop();
        });

    Logger::info("Server Plugin built");
}

// clang-format on
