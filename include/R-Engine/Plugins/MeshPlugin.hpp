#pragma once

#include <R-Engine/Core/Flagable.hpp>
#include <R-Engine/Plugins/Plugin.hpp>
#include <R-Engine/Types.hpp>

namespace r {

// clang-format off

enum class MeshUploadMode : u8 {
    ENABLE_GPU_UPLOAD       = 1 << 0,
    ENABLE_CPU_UPLOAD       = 1 << 1,
    AUTO_COMPUTE_NORMALS    = 1 << 2,
    AUTO_COMPUTE_TANGENTS   = 1 << 3,

    DEFAULT                 = ENABLE_GPU_UPLOAD
};
R_ENUM_FLAGABLE(MeshUploadMode)

/**
 * @brief configuration for the MeshPlugin
 */
struct R_ENGINE_API MeshPluginConfig {
    public:
        MeshUploadMode mode{MeshUploadMode::DEFAULT};
};

/**
 * @brief plugin that manages mesh assets and rendering
 * @details provides systems for:
 *   - managing mesh assets (CPU & GPU)
 *   - uploading dirty meshes to GPU
 *   - rendering mesh components
 *   - asset cleanup
 */
class R_ENGINE_API MeshPlugin final : public Plugin
{
    public:
        explicit MeshPlugin(const MeshPluginConfig &config = MeshPluginConfig()) noexcept;

        /**
         * @brief build the plugin into the application
         * @param app the application to add systems and resources to
         */
        void build(Application &app) noexcept override;

    private:
        MeshPluginConfig _config;
};

// clang-format on
}// namespace r
