#pragma once

#include <R-Engine/Plugins/Plugin.hpp>

namespace r {

class MeshPlugin final : public Plugin
{
    public:
        MeshPlugin() = default;
        ~MeshPlugin() = default;

        void build(Application &app) noexcept override;

    private:
};

}// namespace r
