#pragma once

#include <R-Engine/R-EngineExport.hpp>

#include <R-Engine/Plugins/Plugin.hpp>
#include <R-Engine/Types.hpp>
#include <R-Engine/UI/Theme.hpp>
#include <R-Engine/UI/Events.hpp>

#include <string>
#include <vector>

namespace r {

// clang-format off

// UiTheme and UiEvents moved into include/R-Engine/UI/* headers

/**
 * @brief Runtime config for UiPlugin (debug toggles, etc.).
 */
struct R_ENGINE_API UiPluginConfig {
    bool show_debug_overlay = true;
    std::string overlay_text = "UI Plugin Ready";
};

// clang-format on

/**
 * @brief UI plugin providing resources and systems for UI.
 * @details Skeleton version: inserts theme + events resources and adds empty
 *          systems for STARTUP/UPDATE/RENDER. RENDER draws a tiny overlay
 *          when enabled to validate integration.
 */
class R_ENGINE_API UiPlugin final : public Plugin {
    public:
        explicit UiPlugin(const UiPluginConfig &cfg = UiPluginConfig()) noexcept;
        ~UiPlugin() override = default;

        void build(Application &app) override;

    private:
        UiPluginConfig _config;
};

} // namespace r
