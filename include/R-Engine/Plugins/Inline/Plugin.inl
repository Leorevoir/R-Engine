#pragma once

#include "R-Engine/Plugins/Plugin.hpp"
#include <type_traits>
#include <utility>

template<typename PluginT>
r::PluginGroup &r::PluginGroup::set(PluginT plugin)
{
    static_assert(std::is_base_of_v<Plugin, PluginT>, "Type T must be a derivative of r::Plugin");

    bool replaced = false;
    for (auto &p : _plugins) {
        if (auto casted = dynamic_cast<PluginT *>(p.get())) {
            p = std::make_unique<PluginT>(std::move(plugin));
            replaced = true;
            break;
        }
    }

    if (!replaced) {
        _plugins.push_back(std::make_unique<PluginT>(std::move(plugin)));
    }
    return *this;
}

template<typename PluginT, typename... Args>
void r::PluginGroup::add(Args &&...args)
{
    static_assert(std::is_base_of_v<Plugin, PluginT>, "Type T must be a derivative of r::Plugin");
    _plugins.push_back(std::make_unique<PluginT>(std::forward<Args>(args)...));
}
