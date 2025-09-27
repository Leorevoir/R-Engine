#pragma once

#include <R-Engine/Assets/Handle.hpp>
#include <R-Engine/Mesh/Mesh.hpp>

namespace r {

/**
* @brief Mesh2d is a 2D mesh
*/
struct R_ENGINE_API Mesh2d {
    public:
        MeshHandle mesh;
        explicit Mesh2d(const MeshHandle m = {});
};

/**
 * @brief 2D transformation component
 */
struct Transform2d {
        Vec2f position{0.f, 0.f};
        f32 rotation = 0.f;
        Vec2f scale{1.f, 1.f};

        Transform2d() = default;

        explicit Transform2d(const Vec2f &pos) : position(pos)
        {
            /* __ctor__ */
        }

        Transform2d(const Vec2f &pos, f32 rot, const Vec2f &sc = {1.f, 1.f}) : position(pos), rotation(rot), scale(sc)
        {
            /* __ctor__ */
        }

        /**
         * @brief Create a transform from position
         */
        static Transform2d from_xy(f32 x, f32 y)
        {
            return Transform2d(Vec2f{x, y});
        }
};

}// namespace r
