#pragma once

#include <R-Engine/Plugins/Plugin.hpp>

namespace r {

class RenderPlugin final : public Plugin
{
    public:
        void build(Application &app) override;

    private:
};

}// namespace r
