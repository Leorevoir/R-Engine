#include "ServerPluginImpl.hpp"

#include "RTypeNet/Subplatform/Disconnect.hpp"

#include <R-Engine/Core/Error.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Core/Networking.hpp>

#include <algorithm>
#include <iomanip>
#include <ranges>
#include <sstream>
#include <string>
#include <system_error>

static std::string ipToStr(const std::array<u8, 16> &b)
{
    bool v4mapped = std::all_of(b.begin(), b.begin() + 10, [](u8 x) { return x == 0; }) && b[10] == 0xff && b[11] == 0xff;
    bool all_zero = std::ranges::all_of(b, [](const u8 x) { return x == 0; });

    if (bool v4compat = !all_zero && std::all_of(b.begin(), b.begin() + 12, [](const u8 x) { return x == 0; }); v4mapped || v4compat) {
        std::ostringstream os;
        os << static_cast<unsigned>(b[12]) << '.' << static_cast<unsigned>(b[13]) << '.' << static_cast<unsigned>(b[14]) << '.'
           << static_cast<unsigned>(b[15]);
        return os.str();
    }

    u16 w[8];
    for (u8 i = 0; i < 8; ++i) {
        w[i] = (static_cast<u16>(b[2 * i] << 8) | b[static_cast<u16>(2 * i + 1)]);
    }

    int bestStart = -1, bestLen = 0;
    for (int i = 0; i < 8;) {
        if (w[i] != 0) {
            ++i;
            continue;
        }
        int j = i;
        while (j < 8 && w[j] == 0)
            ++j;
        if (int len = j - i; len > bestLen && len >= 2) {
            bestStart = i;
            bestLen = len;
        }
        i = j;
    }

    std::ostringstream os;
    os << std::hex;
    for (int i = 0; i < 8;) {
        if (i == bestStart) {
            if (i == 0)
                os << "::";
            else
                os << ':';
            i += bestLen;
            if (i >= 8)
                break;
        } else {
            if (i != 0 && i != bestStart + bestLen)
                os << ':';
            os << std::nouppercase << std::hex << static_cast<unsigned>(w[i]);
            ++i;
        }
    }

    std::string out = os.str();
    if (out.empty())
        out = "::";
    return out;
}

void r::ServerPluginImpl::start(const ecs::Res<ServerPluginConfig> config)
{
    rtype::network::startup();
    if (!config.ptr->ip.has_value() || !config.ptr->port.has_value()) {
        throw exception::Error("ServerPluginImpl::start", "ServerPlugin requires both IP and port to be set in its configuration");
    }
    const rtype::network::Endpoint e{.ip = config.ptr->ip.value(), .port = config.ptr->port.value()};
    auto protocol = rtype::network::Protocol::UDP;
    if (config.ptr->protocol == ServerPluginProtocol::TCP) {
        protocol = rtype::network::Protocol::TCP;
    }
    try {
        _serverSock = listen(e, protocol);
    } catch (const std::system_error &err) {
        throw exception::Error("ServerPluginImpl::start", err.what());
    }
    _running = true;
    Logger::info("Server started on " + ipToStr(e.ip) + ":" + std::to_string(e.port) + " with protocol "
        + (protocol == rtype::network::Protocol::TCP ? "TCP" : "UDP"));
}

void r::ServerPluginImpl::update()
{
    /* TODO: Handle incoming connections and data */
}

void r::ServerPluginImpl::stop() noexcept
{
    for (auto &sock : _sockets | std::views::values) {
        disconnect(sock);
    }
    _sockets.clear();
    disconnect(_serverSock);
    _running = false;
    rtype::network::cleanup();
    Logger::info("Server stopped");
}
