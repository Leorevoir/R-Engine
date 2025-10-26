#pragma once
#include <R-Engine/Plugins/Plugin.hpp>

class MenuPlugin final : public r::Plugin
{
    public:
        void build(r::Application &app) override;
};
