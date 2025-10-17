#pragma once

#include <R-Engine/Scripts/LuaApiScript.hpp>
#include <memory>
#include <vector>

namespace r {

/**
 * @brief Registry for scriptable API modules.
 */
class LuaApiRegistry final
{
    public:
        /**
         * @brief add a new API module to the registry.
         */
        void add_api(std::shared_ptr<LuaApiScript> api_script);

        /**
         * @brief get all registered API modules.
         */
        const std::vector<std::shared_ptr<LuaApiScript>> &get_apis() const noexcept;

    private:
        std::vector<std::shared_ptr<LuaApiScript>> _apis;
};

}// namespace r
