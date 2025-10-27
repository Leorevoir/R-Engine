#pragma once
#include <R-Engine/Plugins/Plugin.hpp>

class GameplayPlugin final : public r::Plugin
{
    public:
        void build(r::Application &app) override;
};
