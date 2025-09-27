#pragma once

#include <R-Engine/Assets/Handle.hpp>
#include <R-Engine/Maths/Mat.hpp>
#include <R-Engine/Mesh/Mesh.hpp>

namespace r {

/**
 * @brief Camera3d component for 3D rendering
 */
struct R_ENGINE_API Camera3d {
    public:
        f32 fov = 45.0f;
        f32 near_plane = 0.1f;
        f32 far_plane = 1000.0f;

        Camera3d() = default;

        static Camera3d default_camera()
        {
            return Camera3d{};
        }
};

/**
 * @brief Point light component for 3D lighting
 */
struct R_ENGINE_API PointLight {
    public:
        Vec3f color{1.0f, 1.0f, 1.0f};
        f32 intensity = 1.0f;
        f32 range = 10.0f;
        bool shadows_enabled = false;

        PointLight() = default;
};

/**
* @brief Material3d is a texture & shader id placeholder
*/
struct R_ENGINE_API Material3d {
    public:
        u32 texture_id = 0;
        u32 shader_id = 0;
        Color color{255, 255, 255, 255};
};
/**
* @brief Mesh3d is you guess it a 3D mesh
*/
struct R_ENGINE_API Mesh3d {
    public:
        MeshHandle mesh;
        explicit Mesh3d(const MeshHandle m = {});
};

/**
* @brief represents a 3D transformation with position, rotation (in radians), and scale
* @details
* - position: translation vector
* - rotation: Euler angles in radians (pitch, yaw, roll)
* - scale: scaling factors along each axis
*/
struct R_ENGINE_API Transform3d {
    public:
        Transform3d() = default;

        /**
        * @brief creates an identity transform (no translation, no rotation, unit scale)
        */
        static Transform3d Identity() noexcept;

        /**
        * @brief converts the transform to a 4x4 transformation matrix
        */
        Mat4f to_matrix() const noexcept;

        Vec3f position{0.f, 0.f, 0.f};
        Vec3f rotation{0.f, 0.f, 0.f};
        Vec3f scale{1.f, 1.f, 1.f};

    private:
};

}// namespace r
