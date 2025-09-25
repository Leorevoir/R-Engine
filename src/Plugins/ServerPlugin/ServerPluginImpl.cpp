#include "ServerPluginImpl.hpp"

#include "RTypeNet/Subplatform/Disconnect.hpp"

#include <R-Engine/Core/Error.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Core/Networking.hpp>

#include <ranges>
#include <system_error>

void r::ServerPluginImpl::start(const ecs::Res<ServerPluginConfig> config)
{
    if (!config.ptr->ip.has_value() || !config.ptr->port.has_value()) {
        throw exception::Error("ServerPluginImpl::start", "ServerPlugin requires both IP and port to be set in its configuration");
    }
    const rtype::network::Endpoint e{.ip = config.ptr->ip.value(), .port = config.ptr->port.value()};
    auto protocol = rtype::network::Protocol::UDP;
    if (config.ptr->settings == ServerPluginProtocol::TCP) {
        protocol = rtype::network::Protocol::TCP;
    }
    try {
        _serverSock = listen(e, protocol);
    } catch (const std::system_error &err) {
        throw exception::Error("ServerPluginImpl::start", err.what());
    }
    _running = true;
    Logger::info("Server started");
}

void r::ServerPluginImpl::update()
{
    /* TODO: Handle incoming connections and data */
}

void r::ServerPluginImpl::stop() noexcept
{
    for (auto &sock : _sockets | std::views::values) {
        rtype::network::subplatform::disconnect(sock);
    }
    _sockets.clear();
    disconnect(_serverSock);
    _running = false;
    Logger::info("Server stopped");
}
