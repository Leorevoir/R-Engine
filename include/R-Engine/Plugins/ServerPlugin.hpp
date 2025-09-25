#pragma once

#include <R-Engine/Plugins/Plugin.hpp>
#include <R-Engine/R-EngineExport.hpp>
#include <R-Engine/Types.hpp>

#include <array>
#include <optional>

namespace r {

inline constexpr u8 ip_bytes_len = 16;

// clang-format off

R_ENGINE_API std::array<u8, ip_bytes_len> buildIpv4(const std::array<u8, 4> &b) noexcept;

enum class ServerPluginProtocol : u16 {
    TCP     = 1 << 0,
    UDP     = 1 << 1,

    DEFAULT = TCP
};

struct R_ENGINE_API ServerPluginConfig {
    std::optional<u16> port = std::nullopt;
    std::optional<std::array<u8, ip_bytes_len>> ip = std::nullopt;
    ServerPluginProtocol protocol = ServerPluginProtocol::DEFAULT;
};

// clang-format on

class R_ENGINE_API ServerPlugin final : public Plugin
{
    public:
        explicit ServerPlugin(const ServerPluginConfig &config);

        void build(Application &app) override;

    private:
        ServerPluginConfig _config;
};

}// namespace r
