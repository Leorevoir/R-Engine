#pragma once

#include <R-Engine/Plugins/Plugin.hpp>

namespace r {

class R_ENGINE_API TransformPlugin final : public Plugin
{
    public:
        TransformPlugin() = default;
        ~TransformPlugin() override = default;

        void build(Application &app) override;
};

}// namespace r
