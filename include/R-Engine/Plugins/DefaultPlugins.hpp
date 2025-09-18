#pragma once

#include <R-Engine/Plugins/Plugin.hpp>

namespace r {

/**
 * @brief This group of plugins adds all the default features
 * for an R-Engine application.
 *
 * You can configure the plugins in this group using the `.set()` method,
 * similar to Bevy's API.
 *
 * Example:
 * .add_plugins(DefaultPlugins{}.set(WindowPlugin{
 *     WindowPluginConfig { .title = "My Game" }
 * }))
 */
class R_ENGINE_API DefaultPlugins final : public PluginGroup
{
    public:
        DefaultPlugins();
        void build(Application &app) override;
};

}// namespace r
