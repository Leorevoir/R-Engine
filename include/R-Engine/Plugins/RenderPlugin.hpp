#pragma once

#include <R-Engine/Plugins/Plugin.hpp>
#include <R-Engine/R-EngineExport.hpp>

namespace r {

class R_ENGINE_API RenderPlugin final : public Plugin
{
    public:
        void build(Application &app) override;

    private:
};

}// namespace r
