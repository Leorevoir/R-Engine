#include <R-Engine/Scripts/LuaApiRegistry.hpp>

/**
* public
*/

void r::LuaApiRegistry::add_api(std::shared_ptr<LuaApiScript> api_script)
{
    if (api_script) {
        _apis.push_back(std::move(api_script));
    }
}

const std::vector<std::shared_ptr<r::LuaApiScript>> &r::LuaApiRegistry::get_apis() const noexcept
{
    return _apis;
}
