#pragma once

#include <R-Engine/Plugins/Plugin.hpp>

namespace r {

/**
 * @brief This group of plugins adds all the default features
 * for an R-Engine application.
 * 
 * The inclusion of plugins is controlled by macros in R-Engine/Config.hpp,
 * mimicking the Cargo “features” system in Bevy.
 */
class DefaultPlugins final : public PluginGroup
{
    public:
        void build(Application &app) override;
};

}// namespace r
