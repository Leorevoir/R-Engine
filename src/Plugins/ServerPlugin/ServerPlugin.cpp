#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Plugins/ServerPlugin.hpp>

#include "ServerPluginImpl.hpp"

std::array<u8, r::ip_bytes_len> r::buildIpv4(const std::array<u8, 4> &b) noexcept
{
    return {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, b[0], b[1], b[2], b[3]};
}

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
