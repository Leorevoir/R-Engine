#pragma once

#include <R-Engine/Core/Networking.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/Plugins/ServerPlugin.hpp>
#include <R-Engine/R-EngineExport.hpp>
#include <R-Engine/Types.hpp>

#include <atomic>
#include <unordered_map>

namespace r {

class R_ENGINE_LOCAL ServerPluginImpl final : public NonCopyable
{
    public:
        constexpr explicit ServerPluginImpl() = default;
        constexpr ~ServerPluginImpl() noexcept = default;

        void start(ecs::Res<ServerPluginConfig> config);
        void update();
        void stop() noexcept;

    private:
        rtype::network::Socket _serverSock{.handle = rtype::network::INVALID_SOCK};
        std::unordered_map<u32, rtype::network::Socket> _sockets{};
        std::atomic<bool> _running = false;
        std::atomic<u32> _nextId = 0;
};

}// namespace r
